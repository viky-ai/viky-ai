class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :validate_owner_and_agent
  before_action :build_log_request
  before_action :check_agent_token

  def interpret
    @nlp = Nlp::Interpret.new(interpret_parameters.except(:context))

    @nlp.agent_token = request.headers["Agent-Token"]     if @nlp.agent_token.blank?
    @nlp.language    = request.headers["Accept-Language"] if @nlp.language.blank?

    respond_to do |format|
      format.json {
        if @nlp.valid?
          body, status = request_nlp
        else
          status = 422
          body = { errors: @nlp.errors.full_messages }
          @log.with_response(status, body).save
        end
        render json: body, status: status unless status == 200
      }
    end
  end


  private

    def request_nlp
      begin
        @response = @nlp.proceed
        status = @response[:status].to_i
        body = @response[:body]
        log_body = body
      rescue EOFError => e
        # NLP has crashed
        status   = 503
        body     = { errors: [t('nlp.unavailable')] }
        log_body = { errors: [t('nlp.unavailable'), 'NLP have just crashed'] }
      rescue Errno::ECONNREFUSED => e
        # no more NLP is running
        status   = 503
        body     = { errors: [t('nlp.unavailable')] }
        log_body = { errors: [t('nlp.unavailable'), 'No more NLP server are available', e.message] }
      rescue => e
        # unexpected error
        status   = 500
        log_body = { errors: [t('nlp.unavailable'), e.inspect] }
        raise
      ensure
        @log.with_response(status, log_body).save
      end
      [body, status]
    end

    # Auto render 404.json on ActiveRecord::RecordNotFound exception
    def validate_owner_and_agent
      @owner = User.friendly.find(params[:ownername])
      @agent = Agent.owned_by(@owner).friendly.find(params[:agentname])
    end

    def check_agent_token
      agent_token = params[:agent_token] || request.headers["Agent-Token"]
      if @agent.api_token != agent_token
        status = 401
        body = { errors: [t('controllers.api.access_denied')] }
        @log.with_response(status, body).save
        render json: body, status: status
      end
    end

    def interpret_parameters
      params.permit(
        :ownername,
        :agentname,
        :format,
        :sentence,
        :language,
        :agent_token,
        :verbose,
        :now,
        :spellchecking,
        context: {}
      )
    end

    def build_log_request
      parameters = interpret_parameters

      @log = InterpretRequestLog.new(
        timestamp: Time.now.iso8601(3),
        sentence: parameters['sentence'],
        language: parameters['language'],
        spellchecking: parameters['spellchecking'],
        now: parameters['now'],
        agent: @agent,
        context: parameters['context']
      )
    end
end
