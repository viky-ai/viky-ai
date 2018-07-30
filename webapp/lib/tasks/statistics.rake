namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |t, args|
    environments = [Rails.env]
    environments << 'test' if Rails.env == 'development'
    environments.each do |environment|
      puts Rainbow("Environment #{environment}.")
      client = IndexManager.client environment
      IndexManager.fetch_template_configurations.each do |template_conf|
        active_template = StatisticsIndexTemplate.new template_conf
        save_template(client, active_template) unless template_exists?(client, active_template)
        inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
        save_template(client, inactive_template) unless template_exists?(client, inactive_template)
        next if index_exists?(client, active_template)
        index = StatisticsIndex.from_template active_template
        create_index(client, index)
        update_index_aliases(client, [
          { add: { index: index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }
        ],
          InterpretRequestLog::INDEX_ALIAS_NAME)
      end
    end
  end


  desc 'Reindex the specified statistics index into a new one'
  task :reindex, [:src_index] => :environment do |t, args|
    puts Rainbow('Missing param: src index').red unless args.src_index.present?
    src_name = args.src_index
    client = IndexManager.client
    unless client.indices.exists? index: src_name
      puts Rainbow("Source index #{src_name} does not exists.")
      exit 1
    end
    template_conf = IndexManager.fetch_template_configurations('template-stats-interpret_request_log').first
    active_template = StatisticsIndexTemplate.new template_conf
    save_template(client, active_template)
    inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
    save_template(client, inactive_template)
    src_index = StatisticsIndex.from_name src_name
    template = src_index.active? ? active_template : inactive_template
    if src_index.need_reindexing? template
      dest_index = StatisticsIndex.from_template template
      reindex_into_new(client, src_index, dest_index)
    else
      puts Rainbow("No need to reindex #{src_index.name} : skipping.")
    end
  end


  namespace :reindex do
    desc 'Reindex all statistics indices'
    task :all => :environment do |t, args|
      client = IndexManager.client
      template_conf = IndexManager.fetch_template_configurations('template-stats-interpret_request_log').first
      active_template = StatisticsIndexTemplate.new template_conf
      save_template(client, active_template)
      inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
      save_template(client, inactive_template)
      indices = client.indices.get(index: 'stats-*').keys
                              .map { |index_name| StatisticsIndex.from_name index_name }
                              .partition { |index| index.active? }
      active_indices = indices.first.select { |index| index.need_reindexing? active_template }
      inactive_indices = indices.second.select { |index| index.need_reindexing? inactive_template }
      if (active_indices + inactive_indices).empty?
        puts Rainbow('Nothing to reindex.')
        exit 0
      end
      puts Rainbow("Reindex indices #{(active_indices.map(&:name) + inactive_indices.map(&:name))}.")
      active_indices.each do |src_index|
        dest_index = StatisticsIndex.from_template active_template
        reindex_into_new(client, src_index, dest_index)
      end
      inactive_indices.each do |src_index|
        dest_index = StatisticsIndex.from_template inactive_template
        reindex_into_new(client, src_index, dest_index)
      end
    end
  end


  desc 'Roll over index if older than 7 days or have more than 100 000 documents'
  task :rollover => :environment do |t, args|
    client = IndexManager.client
    template_conf = IndexManager.fetch_template_configurations('template-stats-interpret_request_log').first
    active_template = StatisticsIndexTemplate.new template_conf
    dest_index = StatisticsIndex.from_template active_template
    res = client.indices.rollover(alias: InterpretRequestLog::INDEX_ALIAS_NAME, new_index: dest_index.name, body:{
      conditions: {
        max_age: '7d',
        max_docs: 100_00,
      }
    })
    unless res['rolled_over']
      puts Rainbow('No need to roll over because no condition reached.')
      exit 0
    end
    old_index = res['old_index']
    puts Rainbow("Index #{old_index} rolled over to #{dest_index.name}.")
    shrink_node_name = pick_random_node(client)
    client.indices.put_settings index: old_index, body: {
      'index.routing.allocation.require._name' => shrink_node_name,
      'index.blocks.write' => true
    }
    puts Rainbow("Index #{old_index} switched to read only and migrating to #{shrink_node_name}.")
    client.cluster.health wait_for_no_relocating_shards: true
    puts Rainbow('Shards migration completed.')
    inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
    target_name = StatisticsIndex.from_template inactive_template
    client.indices.shrink index: old_index, target: target_name.name
    puts Rainbow("Index #{old_index} shrink into #{target_name.name}.")
    client.indices.forcemerge index: target_name.name, max_num_segments: 1
    puts Rainbow("Index #{target_name.name} force merged.")
    client.indices.put_settings index: target_name.name, body: {
      'index.number_of_replicas' => 1
    }
    puts Rainbow("Configured a replica for index #{target_name.name}.")
    update_index_aliases(client, [
      { add: { index: target_name.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } },
      { remove: { index: old_index, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }
    ],
      InterpretRequestLog::SEARCH_ALIAS_NAME)
    client.indices.delete index: old_index
    puts Rainbow("Old index #{old_index} deleted.")
  end


  private
    def template_exists?(client, template)
      if client.indices.exists_template? name: template.name
        puts Rainbow("Template #{template.name} already exists : skipping.")
        return true
      end
      false
    end

    def save_template(client, template)
      client.indices.put_template name: template.name, body: template.configuration
      puts Rainbow("Save index template #{template.name} succeed.").green
    end

    def reindex_into_new(client, src_index, dest_index)
      dest_index.snapshot_id = src_index.snapshot_id if src_index.snapshot?
      create_index(client, dest_index)
      if src_index.active?
        update_index_aliases(client, [
          { remove: { index: src_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } },
          { add: { index: dest_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }
        ],
          InterpretRequestLog::INDEX_ALIAS_NAME)
      else
        client.indices.put_settings index: dest_index.name, body: {
          'index.number_of_replicas' => 1
        }
      end
      if src_index.snapshot?
        client.indices.put_settings index: dest_index.name, body: {
          'index.number_of_replicas' => 0
        }
      end
      puts Rainbow("Reindex #{src_index.name} to #{dest_index.name}.").green
      reindex(client, src_index, dest_index)
      update_index_aliases(client, [
        { remove: { index: src_index.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } },
      ],
        InterpretRequestLog::SEARCH_ALIAS_NAME)
      puts Rainbow("Delete #{src_index.name}.").green
      delete_index(client, src_index)
    end

    def index_exists?(client, template)
      expected_status = Rails.env == 'production' ? 'green' : 'yellow'
      index_present = client.cluster.health(level: 'indices', wait_for_status: expected_status)['indices'].keys.any? do |index|
        index =~ Regexp.new(template.index_patterns, Regexp::IGNORECASE)
      end
      if index_present
        puts Rainbow("Index like #{template.index_patterns} already exists : skipping.")
        return true
      end
      false
    end

    def create_index(client, index)
      client.indices.create index: index.name
      puts Rainbow("Creation of index #{index.name} succeed.").green
    end

    def update_index_aliases(client, actions, index_alias)
      client.indices.update_aliases body: { actions: actions }
      puts Rainbow("Update aliases #{index_alias} succeed.").green
    end

    def reindex(client, src_index, dest_index)
      client.reindex(wait_for_completion: true, body: {
        source: { index: src_index.name },
        dest: { index: dest_index.name }
      })
      puts Rainbow("Reindexing of #{src_index.name} to #{dest_index.name} succeed.").green
    end

    def delete_index(client, src_index)
      client.indices.delete index: src_index.name
      puts Rainbow("Remove previous index #{src_index.name} succeed.").green
    end

    def pick_random_node(client)
      node_stats = client.nodes.info()['nodes']
      shrink_node = node_stats.keys.sample
      node_stats[shrink_node]['name']
    end
end
