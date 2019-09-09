require_relative 'lib/task'

namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |_, _|
    environments = [Rails.env]
    environments << 'test' if Rails.env == 'development'
    environments.each do |environment|
      Task::Print.step("Environment #{environment}.")
      client = InterpretRequestLogClient.long_waiting_client
      unless client.cluster_ready?
        Task::Print.error('Cannot perform tasks : cluster is not ready')
        exit 1
      end

      template_conf = IndexManager.template_configuration
      active_template, inactive_template = client.save_template_configuration(template_conf)
      Task::Print.success("Index templates #{active_template.name}, #{inactive_template.name} saved.")
      Task::Print.substep("Index #{active_template.index_name}.")
      cleanup_broken_setup(client)
      setup_active_index(client, active_template)
    end
    configure_dashboards unless Rails.env == 'test'
  end


  desc 'Reindex the specified statistics index into a new one'
  task :reindex, [:src_index] => :environment do |_, args|
    Task::Print.error('Missing param: src index') unless args.src_index.present?
    src_name = args.src_index
    client = InterpretRequestLogClient.long_waiting_client
    unless client.cluster_ready?
      Task::Print.error('Cannot perform tasks : cluster is not ready')
      exit 1
    end
    unless client.index_exists? src_name
      Task::Print.error("Source index #{src_name} does not exists.")
      exit 1
    end

    template_conf = IndexManager.template_configuration
    active_template, inactive_template = client.save_template_configuration template_conf
    current_index = StatisticsIndex.from_name src_name
    template = current_index.active? ? active_template : inactive_template
    if current_index.need_reindexing? template
      new_index = StatisticsIndex.from_template template
      reindex_into_new(client, current_index, new_index)
    else
      Task::Print.notice("No need to reindex #{current_index.name} : skipping.")
    end
  end


  namespace :reindex do
    desc 'Reindex all statistics indices'
    task :all => :environment do |_, _|
      client = InterpretRequestLogClient.long_waiting_client
      unless client.cluster_ready?
        Task::Print.error('Cannot perform tasks : cluster is not ready')
        exit 1
      end

      template_conf = IndexManager.template_configuration
      active_template, inactive_template = client.save_template_configuration template_conf
      indices = client
                  .list_indices
                  .select { |index| index.need_reindexing? active_template }
                  .sort_by { |index| index.state }
                  .map do |index|
                    template = index.active? ? active_template : inactive_template
                    {
                      current_index: index,
                      new_index: StatisticsIndex.from_template(template)
                    }
                  end
      if indices.present?
        Task::Print.step("Reindex indices #{indices.map { |conf| conf[:current_index].name }}.")
        indices.each do |index_conf|
          reindex_into_new(client, index_conf[:current_index], index_conf[:new_index])
        end
      else
        Task::Print.notice('Nothing to reindex.')
      end
    end
  end


  desc 'Roll over index if older than 7 days or have more than 100 000 documents'
  task :rollover => :environment do |t, args|
    max_age = '7d'
    max_docs = 100_000
    Task::Print.step("Roll over alias #{InterpretRequestLog::INDEX_ALIAS_NAME} with conditions max_age=#{max_age} or max_docs=#{max_docs}.")
    client = IndexManager.long_waiting_client
    unless cluster_ready?(client)
      Task::Print.error('Cannot perform tasks : cluster is not ready')
      exit 1
    end

    template_conf = IndexManager.template_configuration
    active_template = StatisticsIndexTemplate.new template_conf
    dest_index = StatisticsIndex.from_template active_template
    res = client.indices.rollover(alias: InterpretRequestLog::INDEX_ALIAS_NAME, new_index: dest_index.name, body:{
      conditions: {
        max_age: max_age,
        max_docs: max_docs,
      }
    })
    unless res['rolled_over']
      Task::Print.notice('No need to roll over because no condition reached.')
      exit 0
    end
    old_index = res['old_index']
    Task::Print.substep("Index #{old_index} rolled over to #{dest_index.name}.")
    shrink_node_name = pick_random_node(client)
    client.indices.put_settings index: old_index, body: {
      'index.routing.allocation.require._name' => shrink_node_name,
      'index.blocks.write' => true
    }
    Task::Print.substep("Index #{old_index} switched to read only and migrating to #{shrink_node_name}.")
    client.cluster.health wait_for_no_relocating_shards: true
    Task::Print.substep('Shards migration completed.')
    inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
    target_name = StatisticsIndex.from_template inactive_template
    client.indices.shrink index: old_index, target: target_name.name
    Task::Print.substep("Index #{old_index} shrink into #{target_name.name}.")
    client.indices.forcemerge index: target_name.name, max_num_segments: 1
    Task::Print.substep("Index #{target_name.name} force merged.")
    client.indices.put_settings index: target_name.name, body: {
      'index.number_of_replicas' => 1
    }
    Task::Print.substep("Configured a replica for index #{target_name.name}.")
    update_index_aliases(client, [
      { add: { index: target_name.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } },
      { remove: { index: old_index, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }
    ],
      InterpretRequestLog::SEARCH_ALIAS_NAME)
    delete_index(client, old_index)
  end


  private

    def cleanup_broken_setup(client)
      alias_name = InterpretRequestLog::INDEX_ALIAS_NAME
      if client.index_exists? alias_name
        Task::Print.notice("Delete index with same name as alias (#{alias_name}) because it should not exists.")
        client.delete_index alias_name
      end
    end

    def setup_active_index(client, active_template)
      index_with_valid_regexp_pattern = "#{active_template.index_patterns[0..-2]}.*"
      if client.index_exists? index_with_valid_regexp_pattern
        Task::Print.notice("Index like #{active_template.index_patterns} already exists : skipping index creation.")
      else
        new_index = client.create_active_index active_template
        Task::Print.success("Index #{new_index.name} saved.")
      end
    end

    def configure_dashboards
      Task::Print.step('Configure Kibana.')
      begin
        StatisticsVisualizer.new.configure
      rescue RuntimeError => e
        Task::Print.error(e)
      end
    end

    def reindex_into_new(client, current_index, new_index)
      Task::Print.step("Reindex #{current_index.name} to #{new_index.name}.")
      client.create_index new_index
      Task::Print.success("Index #{new_index.name} created.")
      client.switch_indexing_to_new_active(current_index, new_index) if current_index.active?
      Task::Print.substep("Reindexing of #{current_index.name} to #{new_index.name} in progress...")
      result = client.reindex current_index, new_index
      Task::Print.success("Reindexing of #{current_index.name} to #{new_index.name} succeed.")
      Task::Print.notice("Reindexing result : #{result.to_json}")
      client.remove_unused_index current_index
      Task::Print.success("Index #{current_index.name} removed.")
    end

    def update_index_aliases(client, actions, index_alias)
      client.indices.update_aliases body: { actions: actions }
      Task::Print.success("Update aliases #{index_alias} succeed.")
    end

    def delete_index(client, index_name)
      client.indices.delete index: index_name
      Task::Print.success("Remove index #{index_name} succeed.")
    end

    def pick_random_node(client)
      node_stats = client.nodes.info()['nodes']
      shrink_node = node_stats.keys.sample
      node_stats[shrink_node]['name']
    end

    def cluster_ready?(client)
      expected_status = Rails.env.production? ? 'green' : 'yellow'
      retry_count = 0
      max_retries = 3
      cluster_ready = false
      timetout = '30s'
      while !cluster_ready && retry_count < max_retries do
        Task::Print.substep("Wait for #{timetout} statistics cluster to be ready...")
        begin
          client.cluster.health(wait_for_status: expected_status, timeout: timetout)
          cluster_ready = true
        rescue Elasticsearch::Transport::Transport::Errors::RequestTimeout => e
          retry_count += 1
          Task::Print.notice("Cluster is not ready for the #{retry_count}/#{max_retries} attempt")
        end
      end
      cluster_ready
    end
end
