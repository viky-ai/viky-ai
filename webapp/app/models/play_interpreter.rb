class PlayInterpreter
  include ActiveModel::Model
  include ActiveModel::Validations::Callbacks

  before_validation :set_defaults

  attr_accessor :ownername, :agentname, :text, :language, :spellchecking, :agent, :agents, :results

  validates_presence_of :ownername, :agentname, :text
  validates :text, byte_size: { maximum: 1024 * 8 }

  def proceed
    self.results = {}
    agents.each do |agent|
      request_params = {
        format: "json",
        ownername: agent.owner.username,
        agentname: agent.agentname,
        agent_token: agent.api_token,
        language: language,
        spellchecking: spellchecking,
        verbose: 'false',
        sentence: text,
        enable_list: 'true'
      }
      body, status = Nlp::Interpret.new(request_params).proceed
      self.results[agent.id] = PlayInterpreterResult.new(status, body)
    end
  end

  def result(search_agent)
    results.nil? ? nil : results[search_agent.id]
  end

  def agent_result
    result(agent)
  end


  private

  def set_defaults
    @spellchecking = 'low' if spellchecking.blank?
    @language = "*"       if language.blank?
    @agents  = []         if agents.nil?
    @results = {}         if results.nil?
  end

end

