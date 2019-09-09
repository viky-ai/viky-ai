class InterpretRequestLogClient

  def self.long_waiting_client
    InterpretRequestLogClient.new(
      transport_options: {
        request: { timeout: 5.minutes }
      }
    )
  end

  def initialize(options = {})
    environment = Rails.env
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys

    raise "Missing statistics configuration for environment #{environment}" if config.empty?

    @client = Elasticsearch::Client.new(config[:client].symbolize_keys.merge(options))
  end

  def cluster_ready?
    expected_status = Rails.env.production? ? 'green' : 'yellow'
    retry_count = 0
    max_retries = 3
    cluster_ready = false
    timetout = '30s'
    while !cluster_ready && retry_count < max_retries do
      begin
        @client.cluster.health(wait_for_status: expected_status, timeout: timetout)
        cluster_ready = true
      rescue Elasticsearch::Transport::Transport::Errors::RequestTimeout => _e
        retry_count += 1
      end
    end
    cluster_ready
  end

  def save_template_configuration(configuration)
    active_template = StatisticsIndexTemplate.new configuration, 'active'
    inactive_template = StatisticsIndexTemplate.new configuration, 'inactive'
    [active_template, inactive_template]
      .select { |template| template_exists?(template) }
      .each do |template|
      @client.indices.put_template name: template.name, body: template.configuration
    end
    return active_template, inactive_template
  end

  def template_exists?(template)
    @client.indices.exists_template? name: template.name
  end

  def index_exists?(name_pattern)
    expected_status = Rails.env.production? ? 'green' : 'yellow'
    @client.cluster.health(level: 'indices', wait_for_status: expected_status)['indices'].keys.any? do |index|
      index =~ /^#{name_pattern}$/i
    end
  end

  def create_active_index(active_template)
    index = StatisticsIndex.from_template active_template
    @client.indices.create index: index.name
    @client.indices.update_aliases body: { actions: [{ add: { index: index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }] }
    index
  end

  def create_index(new_index)
    @client.indices.create index: new_index.name
    new_index
  end

  def delete_index(index)
    name = index.is_a?(String) ? index : index.name
    @client.indices.delete index: name
  end

  def remove_unused_index(index)
    begin
      @client.indices.update_aliases body: { actions: [{ remove: { index: index.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }] }
    rescue Elasticsearch::Transport::Transport::Errors::NotFound => _e
      # alias does not exist
    end
    delete_index index
  end

  def reindex(current_index, new_index)
    result = @client.reindex(
      wait_for_completion: true,
      body: {
        source: { index: current_index.name },
        dest: { index: new_index.name }
      }
    )
    enable_replication new_index if new_index.inactive?
    result
  end

  def enable_replication(index)
    @client.indices.put_settings index: index.name, body: {
      'index.number_of_replicas' => 1
    }
  end

  def switch_indexing_to_new_active(current_index, new_index)
    @client.indices.update_aliases body: { actions: [
      { remove: { index: current_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } },
      { add: { index: new_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }
    ] }
  end

  def list_indices
    @client.indices.get(index: 'stats-interpret_request_log-*').keys
                   .map { |name| StatisticsIndex.from_name name }
  end
end
