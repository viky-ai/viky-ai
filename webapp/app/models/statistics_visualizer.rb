require 'net/http'

class StatisticsVisualizer

  JSON_HEADERS = { 'Content-Type' => 'application/json', 'kbn-xsrf' => 'true' }

  def initialize
    uri = URI.parse(endpoint)
    @client = Net::HTTP.new(uri.host, uri.port)
  end

  def endpoint
    ENV.fetch('VIKYAPP_STATISTICS_VISUALIZER_URL') { 'http://localhost:5601' }
  end

  def configure
    kibana_configurations = fetch_configurations
    result = save(clean(kibana_configurations))
    if result.code != '200'
      raise "Error while saving Kibana configuration : #{result.body}"
    end
  end

  private
    def fetch_configurations
      config_file = "#{Rails.root}/config/kibana/kibana-conf.json"
      JSON.parse(ERB.new(File.read(config_file)).result)
    end

    def save(parameters)
      path = '/kibana/api/saved_objects/_bulk_create?overwrite=true'
      @client.post(path, parameters.to_json, JSON_HEADERS)
    end

    def clean(configurations)
      configurations.map do |configuration|
        configuration['id'] = configuration['_id']
        configuration.delete '_id'
        configuration['type'] = configuration['_type']
        configuration.delete '_type'
        configuration['attributes'] = configuration['_source']
        configuration.delete '_source'
        configuration.delete  '_migrationVersion'
        configuration
      end
    end
end
