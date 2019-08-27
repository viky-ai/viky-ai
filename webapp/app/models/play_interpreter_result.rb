class PlayInterpreterResult
  attr_accessor :status, :body

  def initialize(status, body)
    @status = status
    @body = body
  end

  def succeed?
    status == 200
  end

  def failed?
    !succeed?
  end

  def error_messages
    if failed?
      body[:errors].join(', ') unless body[:errors].nil?
    end
  end

  def interpretations_count
    body['interpretations'].size
  end

  def interpretations_count_for_agent(agent_id)
    body['interpretations'].select{|interpretation| interpretation["package"] == agent_id }.size
  end

end
