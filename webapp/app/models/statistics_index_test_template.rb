class StatisticsIndexTestTemplate < StatisticsIndexTemplate

  def initialize
    super
    @state = 'active'
    @index_name = "#{@configuration['index_patterns'][0..-3].gsub(/^stats-/, 'tests-')}"
    @configuration['aliases'].keys.each do |aliase|
      value = @configuration['aliases'][aliase]
      new_alias_name = aliase.gsub(/-stats-/, '-tests-')
      @configuration['aliases'][new_alias_name] = value
      @configuration['aliases'].delete(aliase)
    end
    @configuration['index_patterns'] = "#{@index_name}-*"
  end
end
