class InterpretRequestLog
  include ActiveModel::Model

  INDEX_NAME = 'stats-interpret_request_log'.freeze
  INDEX_ALIAS_NAME = ['index', INDEX_NAME].join('-').freeze
  SEARCH_ALIAS_NAME = ['search', INDEX_NAME].join('-').freeze
  INDEX_TYPE = 'log'.freeze

  attr_accessor :id, :agent, :timestamp, :sentence, :language, :now, :status, :body, :context

  validates :context_to_s, length: {
    maximum: 1000
  }

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
    params.delete(:agent_slug)
    params.delete(:owner_id)
    InterpretRequestLog.new params
  end

  def initialize(attributes = {})
    if attributes[:agent].blank?
      attributes[:agent] = Agent.find(attributes[:agent_id])
      attributes.delete(:agent_id)
    end
    super
    @agent ||= Agent.find(attributes[:agent_id])
    @sentence ||= ''
    @context ||= {}
    @context['agent_version'] = @agent.updated_at
  end

  def with_response(status, body)
    @status = status
    @body = body
    self
  end

  def save
    return false unless valid?
    refresh = Rails.env == 'test'
    client = IndexManager.client
    result = client.index index: INDEX_ALIAS_NAME, type: INDEX_TYPE, body: to_json, id: @id, refresh: refresh
    @id = result['_id']
  end

  def persisted?
    @id.present?
  end


  private

    def to_json
      result = {
        timestamp: @timestamp,
        sentence: @sentence,
        language: @language,
        agent_id: @agent.id,
        agent_slug: @agent.slug,
        owner_id: @agent.owner.id,
        status: @status,
        body: @body,
        context: @context
      }
      result[:now] = @now if @now.present?
      result
    end

    def context_to_s
      @context.to_s
    end
end
