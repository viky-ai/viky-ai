require 'net/http'

class Nlp::Interpret
  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  include ActiveModel::Model
  include ActiveModel::Validations
  include ActiveModel::Validations::Callbacks

  attr_accessor :ownername, :agentname, :format, :sentence, :language,
                :spellchecking, :agent_token, :verbose, :now, :enable_list, :context

  validates_presence_of :ownername, :agentname, :format, :sentence, :agent_token
  validates :sentence, byte_size: { maximum: 7000 }
  validates_inclusion_of :format, in: %w( json )
  validates_inclusion_of :verbose, in: [ "true", "false" ]
  validates_inclusion_of :spellchecking, in: %w( inactive low medium high ), allow_blank: true
  validates_inclusion_of :enable_list, in: [ "true", "false" ], allow_blank: true
  validate :ownername_and_agentname_consistency
  validate :agent_token_consistency
  validate :now_format

  before_validation :set_default

  def proceed
    if valid?
      begin
        response = send_nlp_request
        status   = response[:status].to_i
        body     = response[:body]
        log_body = body
      rescue EOFError => e
        # NLP has crashed
        status   = 503
        body     = { errors: [I18n.t('nlp.unavailable')] }
        log_body = { errors: [I18n.t('nlp.unavailable'), 'NLP have just crashed'] }
      rescue Errno::ECONNREFUSED => e
        # no more NLP is running
        status   = 503
        body     = { errors: [I18n.t('nlp.unavailable')] }
        log_body = { errors: [I18n.t('nlp.unavailable'), 'No more NLP server are available', e.message] }
      rescue => e
        # unexpected error
        status   = 500
        log_body = { errors: [I18n.t('nlp.unavailable'), e.inspect] }
        raise
      ensure
        save_request_in_elastic(status, log_body)
      end
    else
      if errors[:agent_token].empty?
        status = 422
        body = { errors: errors.full_messages }
      else
        status = 401
        body = { errors: [I18n.t('controllers.api.access_denied')] }
      end
      save_request_in_elastic(status, body)
    end
    [body, status]
  end

  def save_request_in_elastic(status, body)
    log = InterpretRequestLog.new(
      timestamp: Time.now.iso8601(3),
      sentence: sentence,
      language: language,
      spellchecking: spellchecking,
      now: now,
      agent: agent,
      context: context
    )
    log.with_response(status, body).save
  end

  def owner
    User.friendly.find(ownername)
  end

  def agent
    Agent.owned_by(owner).friendly.find(agentname)
  end

  def packages
    AgentGraph.new(agent).to_graph.vertices.collect(&:id)
  end

  def endpoint
    ENV.fetch('VIKYAPP_NLP_URL') { 'http://localhost:9345' }
  end

  def url
    "#{endpoint}/interpret/"
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

    def send_nlp_request
      parameters = nlp_request_params
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

    def nlp_request_params
      p = {
        "Accept-Language"  => language,
        'spellchecking'    => spellchecking.present? ? spellchecking : 'low',
        "primary-package"  => agent.id,
        "packages"         => packages,
        "sentence"         => sentence,
        "show-explanation" => verbose == 'true',
        "show-private"     => verbose == 'true'
      }
      p["enable-list"] = (enable_list == "true")
      p["now"] = now unless now.blank?
      p
    end

end
