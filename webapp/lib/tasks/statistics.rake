require_relative 'lib/task'

namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |_, _|
    Task::Print.step("Environment #{Rails.env}.")
    client = InterpretRequestLogClient.long_waiting_client
    unless client.cluster_ready?
      Task::Print.error('Cannot perform tasks : cluster is not ready')
      exit 1
    end

    active_template = StatisticsIndexTemplate.new 'active'
    client.save_template active_template
    inactive_template = StatisticsIndexTemplate.new 'inactive'
    client.save_template inactive_template
    Task::Print.success("Index templates #{active_template.name}, #{inactive_template.name} saved.")
    cleanup_broken_setup(client)
    setup_active_index(client, active_template)
    if ['development', 'test'].include? Rails.env
      Task::Print.step('Environment test.')
      active_test_template = StatisticsIndexTestTemplate.new
      client.save_template active_test_template
      Task::Print.success("Index templates #{active_test_template.name} saved.")
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

    active_template = StatisticsIndexTemplate.new 'active'
    client.save_template active_template
    inactive_template = StatisticsIndexTemplate.new 'inactive'
    client.save_template inactive_template
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

      active_template = StatisticsIndexTemplate.new 'active'
      client.save_template active_template
      inactive_template = StatisticsIndexTemplate.new 'inactive'
      client.save_template inactive_template
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
  task :rollover => :environment do |_, _|
    max_age = '7d'
    max_docs = 100_000
    Task::Print.step("Roll over alias #{client.index_alias_name} with conditions max_age=#{max_age} or max_docs=#{max_docs}.")
    client = InterpretRequestLogClient.long_waiting_client
    unless client.cluster_ready?
      Task::Print.error('Cannot perform tasks : cluster is not ready')
      exit 1
    end

    active_template = StatisticsIndexTemplate.new 'active'
    new_index = StatisticsIndex.from_template active_template
    res = client.rollover(new_index, max_age, max_docs)
    unless res[:rollover_needed?]
      Task::Print.notice('No need to roll over because no condition reached.')
      exit 0
    end
    old_index = res[:old_index]
    Task::Print.substep("Index #{old_index.name} rolled over to #{new_index.name}.")
    Task::Print.substep("Index #{old_index.name} switched to read only and migrating.")
    shrink_node_name = client.migrate_index_to_other_node old_index
    Task::Print.substep("Shards migration to #{shrink_node_name} completed.")
    target_name = client.decrease_space_consumption old_index
    Task::Print.substep("Index #{target_name.name} disk consumption optimized.")
    client.enable_replication target_name
    Task::Print.substep("Configured a replica for index #{target_name.name}.")
    client.switch_search_to_new_index(old_index, target_name)
    client.delete_index old_index
    Task::Print.success("Index #{old_index.name} removed.")
  end


  private

    def cleanup_broken_setup(client)
      alias_name = client.index_alias_name
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
        Task::Print.success("Index #{new_index.name} created.")
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
      client.switch_indexing_to_new_index(current_index, new_index) if current_index.active?
      Task::Print.substep("Reindexing of #{current_index.name} to #{new_index.name} in progress...")
      result = client.reindex current_index, new_index
      Task::Print.success("Reindexing of #{current_index.name} to #{new_index.name} succeed.")
      Task::Print.notice("Reindexing result : #{result.to_json}")
      client.remove_unused_index current_index
      Task::Print.success("Index #{current_index.name} removed.")
    end
end
