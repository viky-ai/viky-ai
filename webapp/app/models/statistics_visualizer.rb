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
    ['index-pattern', 'config', 'visualization', 'dashboard', 'search', 'timelion-sheet']
      .select { |key| kibana_configurations[key.to_sym].present? }
      .each do |key|
        configurations = kibana_configurations[key.to_sym]
        config = configurations.is_a?(Array) ? configurations : [configurations]
        config.each do |current_configuration|
          id = current_configuration['id']
          result = save(key, id, clean(current_configuration))
          if result.code != '200'
            raise 'Error while saving Kibana configuration'
          end
        end
      end
  end

  private
    def fetch_configurations
      config_dir = "#{Rails.root}/config/kibana"
      kibana_objects = ['index-pattern', 'config', 'visualization', 'dashboard', 'search', 'timelion-sheet']
      result = Hash.new
      Dir.foreach(config_dir)
        .select { |filename| filename.downcase.end_with? '.json' }
        .select { |filename| kibana_objects.include? filename.downcase.gsub(/\.json$/, '') }
        .each { |filename| result[filename.downcase.gsub(/\.json$/, '').to_sym] = JSON.parse(ERB.new(File.read("#{config_dir}/#{filename}")).result) }
      result
    end

    def save(type, id, parameters)
      path = "/api/saved_objects/#{type}/#{id}?overwrite=true"
      @client.post(path, parameters.to_json, JSON_HEADERS)
    end

    def clean(configuration)
      configuration.delete 'id'
      configuration.delete 'type'
      configuration.delete 'updated_at'
      configuration.delete 'version'
      configuration
    end
end
