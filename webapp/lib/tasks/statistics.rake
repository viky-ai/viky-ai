namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |t, args|
    list_environments.each do |environment|
      puts Rainbow("Environment #{environment}.")
      client = IndexManager.client environment
      IndexManager.fetch_template_configurations.each do |template_conf|
        create_template(client, template_conf)
        index_name = IndexManager.build_index_name_from(template_conf)
        create_index(client, index_name, template_conf)
      end
    end
  end


  private
    def list_environments
      config = YAML.load(ERB.new(File.read("#{Rails.root}/config/statistics.yml")).result).symbolize_keys
      config.keys
        .reject { |env| env == :default }
        .map { |env| env.to_s }
    end

    def create_template(client, template_conf)
      template_name = "template-#{template_conf['index_patterns'][0..-3]}"
      if client.indices.exists_template? name: template_name
        puts Rainbow("Template #{template_name} already exists : skipping.")
        return
      end
      begin
        client.indices.put_template name: template_name, body: template_conf
        puts Rainbow("Creation of index template #{template_name} succeed.").green
      rescue
        puts Rainbow("Creation of index template #{template_name} failed.").red
      end
    end

    def create_index(client, index_name, template_conf)
      if client.indices.stats(index: template_conf['index_patterns'])['indices'].present?
        puts Rainbow("Index like #{template_conf['index_patterns']} already exists : skipping.")
        return
      end
      begin
        client.indices.create index: index_name
        puts Rainbow("Creation of index #{index_name} succeed.").green
      rescue
        puts Rainbow("Creation of index #{index_name} failed.").red
      end
    end
end
