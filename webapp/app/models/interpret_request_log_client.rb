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

  def get_document(id)
    @client.get index: InterpretRequestLog::SEARCH_ALIAS_NAME, type: InterpretRequestLog::INDEX_TYPE, id: id
  end

  def save_document(json_document, id)
    refresh = Rails.env == 'test'
    @client.index index: InterpretRequestLog::INDEX_ALIAS_NAME, type: InterpretRequestLog::INDEX_TYPE, body: json_document, id: id, refresh: refresh
  end

  def count_documents(params = {})
    result = @client.count index: InterpretRequestLog::SEARCH_ALIAS_NAME, body: params
    result['count']
  end

  def search_documents(query, size)
    @client.search(
      index: InterpretRequestLog::SEARCH_ALIAS_NAME,
      type: InterpretRequestLog::INDEX_TYPE,
      body: { query: query },
      size: size
    )
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

  def ping_cluster
    @client.ping
  end

  def list_cluster_hosts
    @client.transport.hosts
  end

  def save_template_configuration(configuration)
    active_template = StatisticsIndexTemplate.new configuration, 'active'
    inactive_template = StatisticsIndexTemplate.new configuration, 'inactive'
    [active_template, inactive_template]
      .each do |template|
        @client.indices.put_template name: template.name, body: template.configuration
      end
    return active_template, inactive_template
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

  def switch_indexing_to_new_index(current_index, new_index)
    @client.indices.update_aliases body: { actions: [
      { remove: { index: current_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } },
      { add: { index: new_index.name, alias: InterpretRequestLog::INDEX_ALIAS_NAME } }
    ] }
  end

  def switch_search_to_new_index(current_index, new_index)
    @client.indices.update_aliases body: { actions: [
      { remove: { index: current_index.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } },
      { add: { index: new_index.name, alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }
    ] }
  end

  def list_indices
    @client.indices.get(index: 'stats-interpret_request_log-*').keys
                   .map { |name| StatisticsIndex.from_name name }
  end

  def rollover(new_index, max_age, max_docs)
    res = @client.indices.rollover(alias: InterpretRequestLog::INDEX_ALIAS_NAME, new_index: new_index.name, body: {
      conditions: {
        max_age: max_age,
        max_docs: max_docs,
      }
    })
    old_index = StatisticsIndex.from_name res['old_index']
    {
      rollover_needed?: res['rolled_over'],
      old_index: old_index
    }
  end

  def migrate_index_to_other_node(index)
    shrink_node_name = pick_random_node
    @client.indices.put_settings index: index.name, body: {
      'index.routing.allocation.require._name' => shrink_node_name,
      'index.blocks.write' => true
    }
    @client.cluster.health wait_for_no_relocating_shards: true
    shrink_node_name
  end

  def decrease_space_consumption(index, template_conf)
    inactive_template = StatisticsIndexTemplate.new template_conf, 'inactive'
    target_name = StatisticsIndex.from_template inactive_template
    @client.indices.shrink index: index.name, target: target_name.name
    @client.indices.forcemerge index: target_name.name, max_num_segments: 1
    target_name
  end

  def fetch_deployed_index_version(template)
    begin
      current_version = @client.indices.get_template(name: template.name)[template.name]['version']
    rescue
      current_version = 'not_found'
    end
    current_version
  end

  def reset_indices
    active_template, = save_template_configuration(IndexManager.template_configuration)
    full_pattern = active_template.index_patterns.gsub('active-*', '*')
    delete_index full_pattern
    new_index = create_active_index active_template
    @client.indices.flush index: new_index.name
    new_index
  end


  private

    def pick_random_node
      node_stats = @client.nodes.info['nodes']
      shrink_node = node_stats.keys.sample
      node_stats[shrink_node]['name']
    end
end
