class StatisticsIndexTemplate

  attr_reader :configuration, :state

  def initialize(state = 'active')
    @configuration = read_template_configuration
    @state = state
    @index_name = "#{@configuration['index_patterns'][0..-3]}-#{@state}"
    @configuration['index_patterns'] = "#{@index_name}-*"
    if @state == 'inactive'
      @configuration['settings'] = {
        number_of_shards: 1,
        number_of_replicas: 0,
        codec: 'best_compression'
      }
    end
  end

  def name
    "template-#{@index_name}"
  end

  def version
    @configuration['version']
  end

  def index_patterns
    @configuration['index_patterns']
  end


  private

    def read_template_configuration
      template_config_dir = "#{Rails.root}/config/statistics"
      filename = 'template-stats-interpret_request_log.json'
      JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result)
    end
end
