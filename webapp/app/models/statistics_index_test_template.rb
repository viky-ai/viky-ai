class StatisticsIndexTestTemplate < StatisticsIndexTemplate

  def initialize
    super
    @state = 'active'
    @index_name = "#{@configuration['index_patterns'][0..-3].gsub(/^stats-/, 'tests-')}"
    @configuration.delete('aliases')
    @configuration['index_patterns'] = "#{@index_name}-*"
  end
end
