require 'net/http'

class Nlp::Interpret
  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  include ActiveModel::Model
  include ActiveModel::Validations
  include ActiveModel::Validations::Callbacks

  attr_accessor :ownername, :agentname, :format, :sentence, :language, :agent_token, :verbose, :now

  validates_presence_of :ownername, :agentname, :format, :sentence, :agent_token
  validates_inclusion_of :format, in: %w( json )
  validates_inclusion_of :verbose, in: [ "true", "false" ]
  validate :ownername_and_agentname_consistency
  validate :agent_token_consistency
  validate :now_format

  before_validation :set_default

  def proceed
    parameters = nlp_params
    Rails.logger.info "  | Started POST: #{url} at #{Time.now}"
    Rails.logger.info "  | Parameters: #{parameters}"
    uri = URI.parse(url)
    http = Net::HTTP.new(uri.host, uri.port)
    out = http.post(uri.path, parameters.to_json, JSON_HEADERS)
    Rails.logger.info "  | Completed #{out.code}"
    {
      status: out.code,
      body: JSON.parse(out.body)
    }
  end

  def nlp_params
    p = {
      "Accept-Language" => language,
      "packages" => packages,
      "sentence" => sentence,
      "show-explanation" => verbose
    }
    p["now"] = now unless now.blank?
    p
  end

  def endpoint
    ENV.fetch('VOQALAPP_NLP_URL') { 'http://localhost:9345' }
  end

  def url
    "#{endpoint}/interpret/"
  end

  def owner
    User.friendly.find(ownername)
  end

  def agent
    User.friendly.find(ownername).agents.friendly.find(agentname)
  end

  def packages
    AgentGraph.new(agent).to_graph.vertices.collect(&:id)
  end

  private

    def set_default
      self.language = "*"     if language.blank?
      self.verbose  = "false" if verbose.blank?
    end

    def ownername_and_agentname_consistency
      unless ownername.blank? || agentname.blank?
        begin
          owner
        rescue ActiveRecord::RecordNotFound
          errors.add(:ownername, I18n.t(
            'nlp.interpret.invalid_ownername',
            ownername: ownername
          ))
        end
        begin
          agent
        rescue ActiveRecord::RecordNotFound
          errors.add(:agentname, I18n.t(
            'nlp.interpret.invalid_agentname',
            agentname: agentname,
            ownername: ownername
          ))
        end
      end
    end

    def agent_token_consistency
      if errors.full_messages_for(:ownername).empty? && errors.full_messages_for(:agentname).empty?
        if agent.api_token != agent_token
          errors.add :agent_token, I18n.t('nlp.interpret.invalid_agent_token')
        end
      end
    end

    def now_format
      unless self.now.blank?
        begin
          Time.iso8601(self.now)
        rescue ArgumentError => e
          errors.add :now, I18n.t('nlp.interpret.invalid_now_format')
        end
      end
    end

end
