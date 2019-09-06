module IndexManager

  def self.client(options = {})
    environment = Rails.env
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys

    raise "Missing statistics configuration for environment #{environment}" if config.empty?

    Elasticsearch::Client.new(config[:client].symbolize_keys.merge(options))
  end

  def self.long_waiting_client
    IndexManager.client(
      transport_options: {
        request: { timeout: 5.minutes }
      }
    )
  end

  def self.template_configuration
    template_config_dir = "#{Rails.root}/config/statistics"
    filename = 'template-stats-interpret_request_log.json'
    JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result)
  end

  def self.reset_indices
    client = IndexManager.client
    template_conf = template_configuration
    active_template = StatisticsIndexTemplate.new template_conf, 'active'
    client.indices.put_template name: active_template.name, body: template_conf
    inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
    client.indices.put_template name: inactive_template.name, body: template_conf
    new_active_index = renew_index(active_template, client)
    client.indices.update_aliases body: {
      actions: [
        { add: { index: new_active_index.name, alias: active_template.indexing_alias } }
      ]
    }
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
