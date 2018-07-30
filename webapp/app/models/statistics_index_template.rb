class StatisticsIndexTemplate

  attr_reader :configuration, :state

  def initialize(configuration, state = 'active')
    @configuration = configuration.clone
    @state = state
    @configuration['index_patterns'] = "#{index_full_name}-#{@state}-*"
    if @state == 'inactive'
      @configuration[:settings] = {
        number_of_shards: 1,
        number_of_replicas: 0,
        codec: 'best_compression'
      }
    end
  end

  def name
    "template-#{index_full_name}"
  end

  def version
    @configuration['version']
  end

  def index_patterns
    @configuration['index_patterns']
  end

  def index_name
    index_patterns.split('-')[1]
  end

  def indexing_alias
    "index-#{index_full_name.gsub('-active', '')}"
  end

  private
    def index_full_name
      index_patterns[0..-3]
    end
end
