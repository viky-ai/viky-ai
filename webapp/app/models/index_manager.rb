class IndexManager

  def self.client
    config = Rails.application.config_for(:statistics)
    raise "Missing statistics configuration for environment #{Rails.env}" if config.empty?
    Elasticsearch::Client.new(config)
  end
end
