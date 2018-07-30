module IndexManager

  def self.client(environment = Rails.env)
    config = fetch_statistics_configuration(environment)
    Elasticsearch::Client.new(config[:client].symbolize_keys)
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

  def self.build_index_name_from(template)
    uniq_id = SecureRandom.hex(4)
    [template.index_base_name, template.version, uniq_id].join('-')
  end

  def self.reset_indices
    client = IndexManager.client
    fetch_template_configurations.each do |conf|
      ['active', 'inactive'].each do |state|
        template = StatisticsIndexTemplate.new conf, state
        client.indices.put_template name: template.name, body: conf
      end
      new_index = renew_index(template, client)
      client.indices.update_aliases body: {
        actions: [
          { add: { index: new_index, alias: "index-#{template.index_base_name}" } }
        ]
      }
    end
  end

  def self.renew_index(template, client = IndexManager.client)
    full_pattern = template.index_patterns.gsub('active-*', '*')
    client.indices.delete index: full_pattern
    new_name = build_index_name_from(template)
    client.indices.create index: new_name
    client.indices.flush index: new_name
    new_name
  end
end
