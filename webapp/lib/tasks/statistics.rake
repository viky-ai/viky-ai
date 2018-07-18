namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |t, args|
    list_environments = args.environment.present? ? [args.environment] : ['development', 'test']
    list_environments.each do |environment|
      puts Rainbow("Environment #{environment}.")
      client = IndexManager.client environment
      IndexManager.fetch_template_configurations.each do |template_conf|
        save_template(client, template_conf) unless template_exists?(client, template_conf)
        index_name = IndexManager.build_index_name_from(template_conf)
        create_index(client, index_name) unless index_exists?(client, template_conf)
      end
    end
  end


  desc 'Reindex the specified index to a new one'
  task :reindex, [:src_index] => :environment do |t, args|
    puts Rainbow('Missing param: src index').red unless args.src_index.present?
    src_index = args.src_index
    client = IndexManager.client Rails.env
    unless client.indices.exists? index: src_index
      puts Rainbow("Source index #{src_index} does not exists.")
      exit 1
    end
    template_conf = IndexManager.fetch_template_configurations('template-stats-interpret_request_log').first
    save_template(client, template_conf.except('aliases'))
    reindex_into_new(client, src_index, template_conf)
    save_template(client, template_conf)
  end


  desc 'Reindex all indices'
  task :reindex_all => :environment do |t, args|
    client = IndexManager.client Rails.env
    template_conf = IndexManager.fetch_template_configurations('template-stats-interpret_request_log').first
    save_template(client, template_conf.except('aliases'))
    index_indices = client.indices.get_alias(name: 'index-stats-*').keys
    search_indices = client.indices.get_alias(name: 'search-stats-*').keys - index_indices
    index_indices.each do |src_index|
      reindex_into_new(client, src_index, template_conf)
    end
    search_indices.each do |src_index|
      reindex_into_new(client, src_index, template_conf)
    end
    save_template(client, template_conf)
  end


  private
    def template_exists?(client, template_conf)
      template_name = IndexManager.build_template_name_from(template_conf)
      if client.indices.exists_template? name: template_name
        puts Rainbow("Template #{template_name} already exists : skipping.")
        return true
      end
      false
    end

    def save_template(client, template_conf)
      template_name = IndexManager.build_template_name_from(template_conf)
      client.indices.put_template name: template_name, body: template_conf
      puts Rainbow("Save index template #{template_name} succeed.").green
    end

    def reindex_into_new(client, src_index, template_conf)
      dest_index = IndexManager.build_index_name_from template_conf
      create_index(client, dest_index)
      is_used_to_index = check_used_to_index(client, src_index)
      if is_used_to_index
        update_index_aliases(client, [
          { remove: { index: src_index, alias: InterpretRequestLog::INDEX_ALIAS_NAME } },
          { add: { index: dest_index, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }
        ],
          InterpretRequestLog::INDEX_ALIAS_NAME)
      end
      reindex(client, src_index, dest_index)
      update_index_aliases(client, [
        { remove: { index: src_index, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } },
        { add: { index: dest_index, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }
      ],
        InterpretRequestLog::SEARCH_ALIAS_NAME)
      delete_index(client, src_index)
    end

    def index_exists?(client, template_conf)
      if client.indices.stats(index: template_conf['index_patterns'])['indices'].present?
        puts Rainbow("Index like #{template_conf['index_patterns']} already exists : skipping.")
        return true
      end
      false
    end

    def create_index(client, index_name)
      client.indices.create index: index_name
      puts Rainbow("Creation of index #{index_name} succeed.").green
    end

    def check_used_to_index(client, src_index)
      begin
        is_used_to_index = !client.indices.get_alias(index: src_index)[src_index]['aliases']['index-stats-interpret_request_log'].nil?
      rescue
        is_used_to_index = false
      end
      is_used_to_index
    end

    def update_index_aliases(client, actions, index_alias)
      client.indices.update_aliases body: { actions: actions }
      puts Rainbow("Update aliases #{index_alias} succeed.").green
    end

    def reindex(client, src_index, dest_index)
      client.reindex(wait_for_completion: true, body: {
        source: { index: src_index },
        dest: { index: dest_index }
      })
      puts Rainbow("Reindexing of #{src_index} to #{dest_index} succeed.").green
    end

    def delete_index(client, src_index)
      client.indices.delete index: src_index
      puts Rainbow("Remove previous index #{src_index} succeed.").green
    end
end
