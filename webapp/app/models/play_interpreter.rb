class PlayInterpreter
  include ActiveModel::Model
  include ActiveModel::Validations::Callbacks

  attr_accessor :text, :language, :spellchecking, :agents, :result

  validates_presence_of :text, :language, :spellchecking
  validates :text, byte_size: { maximum: 1024 * 8 }

  before_validation :set_defaults
  before_validation :strip_text

  def proceed
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

  def query_default_state?
    text.blank? && language == "*" && spellchecking == "low"
  end


  private

    def strip_text
      self.text = text.strip unless text.nil?
    end

    def set_defaults
      @spellchecking = 'low' if spellchecking.blank?
      @language = "*"       if language.blank?
      @agents  = []         if agents.nil?
    end

end

