class InterpretRequestLogTestClient < InterpretRequestLogClient

  def initialize(options = {})
    super(options)
    @expected_cluster_status = 'yellow'
    @refresh_on_save = true
  end

  def self.index_name
    'tests-interpret_request_log'.freeze
  end

  def index_alias_name
    "#{InterpretRequestLogTestClient.index_name}-active-#{Process.pid}"
  end

  def search_alias_name
    index_alias_name
  end

  def create_test_index
    @client.indices.create index: index_alias_name
  end

  def drop_test_index
    delete_index index_alias_name
  end

  def clear_test_index
    @client.delete_by_query(
      index: index_alias_name,
      body: { query: { match_all: {} } },
      refresh: true,
      wait_for_completion: true
    )
  end
end
