module IndexManager

  def self.client(environment = Rails.env)
    config = fetch_statistics_configuration(environment)
    Elasticsearch::Client.new(config[:client].symbolize_keys)
  end

  def self.cluster_ready?(client)
    expected_status = Rails.env.production? ? 'green' : 'yellow'
    retry_count = 0
    cluster_ready = false
    while !cluster_ready && retry_count < 3 do
      begin
        client.cluster.health(wait_for_status: expected_status)
        cluster_ready = true
      rescue Elasticsearch::Transport::Transport::Errors::RequestTimeout => e
        retry_count += 1
      end
    end
    cluster_ready
  end

  def self.fetch_statistics_configuration(environment = Rails.env)
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys
    raise "Missing statistics configuration for environment #{environment}" if config.empty?
    config
  end

  def self.fetch_template_configurations(filter_name = '')
    template_config_dir = "#{Rails.root}/config/statistics"
    json_list = Dir.foreach(template_config_dir)
                   .select { |filename| filename.downcase.end_with? '.json' }
    json_list.select! { |filename| filename == "#{filter_name}.json" } if filter_name.present?
    json_list.map { |filename| JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result) }
  end

  def self.reset_indices
    client = IndexManager.client
    fetch_template_configurations.each do |conf|
      active_template = StatisticsIndexTemplate.new conf, 'active'
      client.indices.put_template name: active_template.name, body: conf
      inactive_template = StatisticsIndexTemplate.new conf, 'inactive'
      client.indices.put_template name: inactive_template.name, body: conf
      new_active_index = renew_index(active_template, client)
      client.indices.update_aliases body: {
        actions: [
          { add: { index: new_active_index.name, alias: active_template.indexing_alias } }
        ]
      }
    end
  end

  def self.renew_index(template, client = IndexManager.client)
    full_pattern = template.index_patterns.gsub('active-*', '*')
    client.indices.delete index: full_pattern
    new_index = StatisticsIndex.from_template template
    client.indices.create index: new_index.name
    client.indices.flush index: new_index.name
    new_index
  end
end
