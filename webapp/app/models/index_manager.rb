module IndexManager

  def self.client(environment = Rails.env)
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys
    raise "Missing statistics configuration for environment #{environment}" if config.empty?
    Elasticsearch::Client.new(config[:client].symbolize_keys)
  end

  def self.fetch_template_configurations(filter_name = '')
    template_config_dir = "#{Rails.root}/config/statistics"
    json_list = Dir.foreach(template_config_dir)
                   .select { |filename| filename.downcase.end_with? '.json' }
    json_list.select! { |filename| filename == "#{filter_name}.json" } if filter_name.present?
    json_list.map { |filename| JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result) }
  end

  def self.build_index_name_from(template_conf)
    index_base_name = template_conf['index_patterns'][0..-3]
    uniq_id = SecureRandom.hex(4)
    template_version = template_conf['version']
    [index_base_name, template_version, uniq_id].join('-')
  end

  def self.reset_statistics
    client = IndexManager.client
    fetch_template_configurations.each do |conf|
      renew_index(conf, client)
    end
  end

  def self.renew_index(template_conf, client = IndexManager.client)
    index_base_name = template_conf['index_patterns'][0..-3]
    template_version = template_conf['version']
    index_patterns = [index_base_name, template_version, '*'].join('-')
    client.indices.delete index: index_patterns
    new_name = build_index_name_from(template_conf)
    client.indices.create index: new_name
    client.indices.flush index: new_name
  end
end
