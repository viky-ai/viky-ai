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

  def delete_index(index)
    name = index.is_a(String) ? index : index.name
    @client.indices.delete index: name
  end
end
