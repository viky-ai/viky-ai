module IndexManager

  def self.client
    config = Rails.application.config_for(:statistics)
    raise "Missing statistics configuration for environment #{Rails.env}" if config.empty?
    Elasticsearch::Client.new(config)
  end

  def self.fetch_template_configurations
    template_config_dir = "#{Rails.root}/config/statistics"
    Dir.foreach(template_config_dir)
      .select { |filename| filename.downcase.end_with? '.json' }
      .map { |filename| JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result) }
  end

  def self.build_index_name_from(template_conf, environment = Rails.env)
    index_base_name = template_conf['template'][0..-3]
    [index_base_name, environment].join('-')
  end

  def self.reset_statistics
    client = IndexManager.client
    fetch_template_configurations.each do |conf|
      index_name = build_index_name_from(conf)
      reset_index(index_name, client)
    end
  end

  def self.reset_index(index_name, client = IndexManager.client)
    client.indices.delete index: index_name
    client.indices.create index: index_name
    client.indices.flush index: index_name
  end
end
