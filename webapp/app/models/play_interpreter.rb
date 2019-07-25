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

      cache_key = [
        "PlayInterpreter",
        request_params[:ownername],
        request_params[:agentname],
        request_params[:sentence],
        request_params[:language],
        request_params[:spellchecking],
        agent.updated_at
      ].join('/')

      body, status = Rails.cache.fetch(cache_key, expires_in: 60.minutes) do
        Nlp::Interpret.new(request_params).proceed
      end
      Rails.cache.delete(cache_key) unless status == 200

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

