class PlayInterpreter
  include ActiveModel::Model
  include ActiveModel::Validations::Callbacks

  attr_accessor :available_agents, :agent_ids, :text, :language, :spellchecking, :result, :current_user

  before_validation :set_defaults
  before_validation :strip_text

  validates_presence_of :text, :language, :spellchecking
  validates :text, byte_size: { maximum: 1024 * 8 }
  validate :available_agents_and_agent_ids_consistency

  def proceed
    return if agents.blank?

    request_params = {
      format: 'json',
      agents: agents,
      language: language,
      spellchecking: spellchecking,
      verbose: 'false',
      no_overlap: 'true',
      sentence: text,
      context: {
        user_id: current_user.id,
        client_type: 'play_ui'
      }
    }
    body, status = Nlp::Interpret.new(request_params).proceed
    @result = PlayInterpreterResult.new(status, body)
  end

  def query_default_state?
    text.blank? && language == "*" && spellchecking == "low"
  end

  def agents
    Agent.where(id: agent_ids).order(name: :asc)
  end


  private

    def strip_text
      self.text = text.strip if text.present?
    end

    def set_defaults
      @spellchecking = 'low'  if spellchecking.blank?
      @language = "*"         if language.blank?
      @available_agents = []  if available_agents.nil?
      @agent_ids = []         if agent_ids.nil?
    end

    def available_agents_and_agent_ids_consistency
      ids = agent_ids & available_agents.collect(&:id)
      @agent_ids = Agent.where(id: ids).pluck(:id)
    end
end
