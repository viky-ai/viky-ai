class PlayInterpreter
  include ActiveModel::Model
  include ActiveModel::Validations::Callbacks

  attr_accessor :available_agents, :agent_ids, :text, :language, :spellchecking, :result

  validates_presence_of :text, :language, :spellchecking
  validates :text, byte_size: { maximum: 1024 * 8 }
  validate :available_agents_and_agent_ids_consistency

  before_validation :set_defaults
  before_validation :strip_text

  def proceed
    if agents.size > 0
      request_params = {
        format: "json",
        ownername: agents.first.owner.username,
        agentname: agents.first.agentname,
        agent_token: agents.first.api_token,
        agent_ids: agents.collect(&:id),
        language: language,
        spellchecking: spellchecking,
        verbose: 'false',
        sentence: text
      }
      body, status = Nlp::Interpret.new(request_params).proceed
      @result = PlayInterpreterResult.new(status, body)
    end
  end

  def query_default_state?
    text.blank? && language == "*" && spellchecking == "low"
  end

  def agents
    Agent.where(id: agent_ids).order(name: :asc)
  end


  private

    def strip_text
      self.text = text.strip unless text.nil?
    end

    def set_defaults
      @spellchecking = 'low'  if spellchecking.blank?
      @language = "*"         if language.blank?
      @available_agents = []  if available_agents.nil?
      @agent_ids = []         if agent_ids.nil?
    end

    def available_agents_and_agent_ids_consistency
      ids = agent_ids.select { |agent| available_agents.collect(&:id).include? agent }
      @agent_ids = Agent.where(id: ids).pluck(:id)
    end

end

