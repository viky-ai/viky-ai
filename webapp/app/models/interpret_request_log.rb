class InterpretRequestLog
  include ActiveModel::Model

  attr_accessor :id, :agents, :timestamp, :sentence, :language, :spellchecking, :now, :status, :body, :context

  validates :context_to_s, length: {
    maximum: 1000
  }

  def self.count(params = {})
    client = InterpretRequestLogClient.build_client
    client.count_documents(params)
  end

  def self.find(id)
    client = InterpretRequestLogClient.build_client
    result = client.get_document id
    params = result['_source'].symbolize_keys
    params[:id] = result['_id']
    params.delete(:agent_slug)
    params.delete(:owner_id)
    InterpretRequestLog.new params
  end

  def initialize(attributes = {})
    if attributes[:agents].blank?
      attributes[:agents] = Agent.where(id: attributes[:agent_id])
      attributes.delete(:agent_id)
    end
    super
    @agents ||= Agent.where(id: attributes[:agent_id])
    @sentence ||= ''
    @context ||= {}
    @context['agent_version'] = @agents.map(&:updated_at)
  end

  def with_response(status, body)
    @status = status
    @body = body
    self
  end

  def save
    return false unless valid?

    client = InterpretRequestLogClient.build_client
    result = client.save_document(to_json, @id)
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
        spellchecking: @spellchecking,
        agent_id: @agents.map(&:id),
        agent_slug: @agents.map(&:slug),
        owner_id: @agents.map { |agent| agent.owner.id },
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
