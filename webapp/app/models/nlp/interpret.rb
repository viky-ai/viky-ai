require 'net/http'

class Nlp::Interpret
  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  include ActiveModel::Model
  include ActiveModel::Validations
  include ActiveModel::Validations::Callbacks

  attr_accessor :agents, :format, :sentence, :language, :spellchecking, :verbose, :now, :context

  validates_presence_of :agents, :format, :sentence
  validates :sentence, byte_size: { maximum: 1024*8 }
  validates_inclusion_of :format, in: %w( json )
  validates_inclusion_of :verbose, in: [ "true", "false" ]
  validates_inclusion_of :spellchecking, in: %w( inactive low medium high ), allow_blank: true
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
      status = 422
      body = { errors: errors.full_messages }
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
      agents: agents,
      context: context
    )
    log.with_response(status, body).save
  end

  def packages
    agents.collect { |agent|
      AgentGraph.new(agent).to_graph.vertices.collect(&:id)
    }.flatten.uniq
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
        "spellchecking"    => spellchecking.present? ? spellchecking : 'low',
        "primary-packages" => agents.collect(&:id),
        "packages"         => packages,
        "sentence"         => sentence,
        "show-explanation" => verbose == 'true',
        "show-private"     => verbose == 'true'
      }
      p["now"] = now unless now.blank?
      p
    end

end
