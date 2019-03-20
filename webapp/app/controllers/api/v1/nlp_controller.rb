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
          begin
            @response = @nlp.proceed
            status = @response[:status]
            body = @response[:body]
            @log.with_response(status, body)
            render json: body, status: status unless status == '200'
          rescue Errno::ECONNREFUSED => e
            status = 503
            body = { errors: [t('nlp.unavailable')] }
            @log.with_response(status, { errors: e.message })
            render json: body, status: status
          end
        else
          status = 422
          body = { errors: @nlp.errors.full_messages }
          @log.with_response(status, {})
          render json: body, status: status
        end
        @log.save
      }
    end
  end


  private

    # Auto render 404.json on ActiveRecord::RecordNotFound exception
    def validate_owner_and_agent
      @owner = User.friendly.find(params[:ownername])
      @agent = @owner.agents.friendly.find(params[:agentname])
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
        context: {}
      )
    end

    def build_log_request
      parameters = interpret_parameters

      @log = InterpretRequestLog.new(
        timestamp: Time.now.iso8601(3),
        sentence: parameters['sentence'],
        language: parameters['language'],
        now: parameters['now'],
        agent: @agent,
        context: parameters['context']
      )
    end
end
