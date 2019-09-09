module IndexManager

  def self.client(options = {})
    environment = Rails.env
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys

    raise "Missing statistics configuration for environment #{environment}" if config.empty?

    Elasticsearch::Client.new(config[:client].symbolize_keys.merge(options))
  end

  def self.template_configuration
    template_config_dir = "#{Rails.root}/config/statistics"
    filename = 'template-stats-interpret_request_log.json'
    JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result)
  end
end
