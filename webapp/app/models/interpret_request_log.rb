class InterpretRequestLog

  INDEX_NAME = 'stats-interpret_request_log'.freeze
  INDEX_ALIAS_NAME = ['index', INDEX_NAME].join('-').freeze
  SEARCH_ALIAS_NAME = ['search', INDEX_NAME].join('-').freeze

  INDEX_TYPE = 'log'.freeze

  attr_reader :id, :timestamp, :sentence, :language, :now, :status, :body, :context, :errors

  def self.count(params = {})
    client = IndexManager.client
    result = client.count index: SEARCH_ALIAS_NAME, body: params
    result['count']
  end

  def self.find(id)
    client = IndexManager.client
    result = client.get index: SEARCH_ALIAS_NAME, type: INDEX_TYPE, id: id
    params = result['_source'].symbolize_keys
    params[:id] = result['_id']
    InterpretRequestLog.new params
  end

  def initialize(params = {})
    @errors = []

    @id = params[:id]
    @agent = params[:agent].present? ? params[:agent] : Agent.find(params[:agent_id])
    @timestamp = params[:timestamp]
    @sentence = params.fetch(:sentence, '')
    @language = params[:language]
    @now = params[:now]
    @status = params[:status]
    @body = params[:body]
    @context = params[:context].present? ? params[:context] : {}
    unless @context['test'].present?
      @context['client_type'] = @context['client_type'].present? ? @context['client_type']: 'bot'
      @context['agent_version'] = @agent.updated_at
    end
  end

  def valid?
    @errors.empty?
  end

  def with_response(status, body)
    @status = status
    @body = body
    self
  end

  def save
    validate_context_size
    if valid?
      refresh = Rails.env == 'test'
      client = IndexManager.client
      result = client.index index: INDEX_ALIAS_NAME, type: INDEX_TYPE, body: to_json, id: @id, refresh: refresh
      @id = result['_id']
    end
    valid?
  end


  private

    def to_json
      {
        timestamp: @timestamp,
        sentence: @sentence,
        language: @language,
        now: @now,
        agent_id: @agent.id,
        agent_slug: @agent.slug,
        owner_id: @agent.owner.id,
        status: @status,
        body: @body,
        context: @context
      }
    end

    def validate_context_size
      @errors << I18n.t('errors.interpret_request_log.context_too_long', count: 1000) if @context.to_s.size > 1000
    end
end
