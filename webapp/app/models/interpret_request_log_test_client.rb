class InterpretRequestLogTestClient < InterpretRequestLogClient

  def self.long_waiting_client
    InterpretRequestLogTestClient.new(
      transport_options: {
        request: { timeout: 5.minutes }
      }
    )
  end

  def initialize(options = {})
    super(options)
    @expected_cluster_status = 'yellow'
    @refresh_on_save = true
  end

  def self.index_name
    'tests-interpret_request_log'.freeze
  end

  def index_alias_name
    "index-#{InterpretRequestLogTestClient.index_name}"
  end

  def search_alias_name
    "search-#{InterpretRequestLogTestClient.index_name}"
  end

  def reset_indices
    active_template = StatisticsIndexTestTemplate.new
    full_pattern = active_template.index_patterns.gsub('active-*', '*')
    delete_index full_pattern
    new_index = create_active_index active_template
    @client.indices.flush index: new_index.name
    new_index
  end
end
