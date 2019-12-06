class InterpretRequestLogClient

  def self.long_waiting_client
    InterpretRequestLogClient.new(
      transport_options: {
        request: { timeout: 5.minutes }
      }
    )
  end

  def self.build_client
    Rails.env.test? ? InterpretRequestLogTestClient.new : InterpretRequestLogClient.new
  end

  def initialize(options = {})
    environment = Rails.env
    config = Rails.application.config_for(:statistics, env: environment).symbolize_keys

    raise "Missing statistics configuration for environment #{environment}" if config.empty?

    @client = Elasticsearch::Client.new(config[:client].symbolize_keys.merge(options))
    @expected_cluster_status = Rails.env.production? ? 'green' : 'yellow'
    @refresh_on_save = false
  end

  def get_document(id)
    @client.get index: search_alias_name, id: id
  end

  def save_document(json_document, id)
    @client.index index: index_alias_name, body: json_document, id: id, refresh: @refresh_on_save
  end

  def count_documents(params = {})
    result = @client.count index: search_alias_name, body: params
    result['count']
  end

  def search_documents(query, size)
    @client.search(
      index: search_alias_name,
      body: { query: query },
      size: size
    )
  end

  def search(body)
    @client.search(
      index: search_alias_name,
      body: body
    )
  end

  def cluster_ready?
    retry_count = 0
    max_retries = 3
    cluster_ready = false
    timetout = '30s'
    while !cluster_ready && retry_count < max_retries do
      begin
        @client.cluster.health(wait_for_status: @expected_cluster_status, timeout: timetout)
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

  def save_template(template)
    @client.indices.put_template name: template.name, body: template.configuration
    template
  end

  def index_exists?(name_pattern)
    @client.cluster.health(level: 'indices', wait_for_status: @expected_cluster_status)['indices'].keys.any? do |index|
      index =~ /^#{name_pattern}$/i
    end
  end

  def create_active_index(active_template)
    index = StatisticsIndex.from_template active_template
    @client.indices.create index: index.name
    @client.indices.update_aliases body: { actions: [{ add: { index: index.name, alias: index_alias_name } }] }
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
      @client.indices.update_aliases body: { actions: [{ remove: { index: index.name, alias: search_alias_name } }] }
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

  def disable_all_replication()
    @client.indices.put_settings index: '_all', ignore_unavailable: true, allow_no_indices: true, body: {
      'index.number_of_replicas' => 0
    }
  end

  def switch_indexing_to_new_index(current_index, new_index)
    @client.indices.update_aliases body: { actions: [
      { remove: { index: current_index.name, alias: index_alias_name } },
      { add: { index: new_index.name, alias: index_alias_name } }
    ] }
  end

  def switch_search_to_new_index(current_index, new_index)
    @client.indices.update_aliases body: { actions: [
      { remove: { index: current_index.name, alias: search_alias_name } },
      { add: { index: new_index.name, alias: search_alias_name } }
    ] }
  end

  def list_indices
    @client.indices.get(index: "#{InterpretRequestLogClient.index_name}-*").keys
                   .map { |name| StatisticsIndex.from_name name }
  end

  def rollover(new_index, max_age, max_docs)
    res = @client.indices.rollover(alias: index_alias_name, new_index: new_index.name, body: {
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

  def decrease_space_consumption(index)
    inactive_template = StatisticsIndexTemplate.new 'inactive'
    target_name = StatisticsIndex.from_template inactive_template
    @client.indices.shrink index: index.name, target: target_name.name
    @client.indices.forcemerge index: target_name.name, max_num_segments: 1
    target_name
  end

  def fetch_deployed_index_version
    template = StatisticsIndexTemplate.new 'active'
    begin
      current_version = @client.indices.get_template(name: template.name)[template.name]['version']
    rescue
      current_version = 'not_found'
    end
    current_version
  end

  def clear_indices
    return if list_indices.blank?

    @client.indices.update_aliases body: { actions: [
      { remove: { index: "#{InterpretRequestLogClient.index_name}-*", alias: index_alias_name } },
      { remove: { index: "#{InterpretRequestLogClient.index_name}-*", alias: search_alias_name } }
    ] }
    list_indices.each { |index| delete_index index }
  end

  def create_local_repository(repository_name, env_name)
    @client.snapshot.create_repository repository: repository_name, body: {
      type: 'fs',
      settings: { location: "/backup_data/#{env_name}/es-backup" }
    }
  end

  def create_s3_restore_repository(repository_name, env_name, bucket = 'viky-ai')
    @client.snapshot.create_repository repository: repository_name, body: {
      type: 's3',
      settings: {
        bucket: bucket,
        readonly: true,
        base_path: "viky-backups/#{env_name}/es-backup"
      }
    }
  end

  def create_snapshot(repository_name, snapshot_name)
    @client.snapshot.create repository: repository_name, snapshot: snapshot_name, wait_for_completion: true, body: {
      indices: "#{InterpretRequestLogClient.index_name}-*",
      include_global_state: false,
      ignore_unavailable: true
    }
  end

  def restore_most_recent_snapshot(repository)
    snapshot = @client.cat.snapshots(
      repository: repository,
      s: 'end_epoch',
      h: 'id'
    ).split("\n").last

    opts = {
      indices: "#{InterpretRequestLogClient.index_name}-*",
      include_aliases: true,
      allow_no_indices: true,
      ignore_unavailable: true,
      index_settings: {}
    }
    if ENV['VIKYAPP_STATISTICS_NO_REPLICA'] == 'true'
      opts[:index_settings]['index.number_of_replicas'] = 0
    end

    @client.snapshot.restore(
      repository: repository,
      snapshot: snapshot,
      wait_for_completion: true,
      body: opts
    )
  end

  def self.index_alias_name
    "index-#{InterpretRequestLogClient.index_name}"
  end

  def index_alias_name
    InterpretRequestLogClient.index_alias_name
  end

  def search_alias_name
    "search-#{InterpretRequestLogClient.index_name}"
  end

  def self.index_name
    'stats-interpret_request_log'.freeze
  end

  private

    def pick_random_node
      node_stats = @client.nodes.info['nodes']
      shrink_node = node_stats.keys.sample
      node_stats[shrink_node]['name']
    end
end
