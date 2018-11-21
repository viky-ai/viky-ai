class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :validate_owner_and_agent
  before_action :build_log_request
  before_action :check_agent_token

  def interpret
    @nlp = Nlp::Interpret.new(interpret_parameters)

    @nlp.agent_token = request.headers["Agent-Token"]     if @nlp.agent_token.blank?
    @nlp.language    = request.headers["Accept-Language"] if @nlp.language.blank?

    respond_to do |format|
      format.json {
        if @nlp.valid?
          @response = @nlp.proceed
          @log.with_response(@response[:status], @response[:body])
          unless @response[:status] == '200'
            render json: @response[:body], status: @response[:status]
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
        :ownername, :agentname, :format, :sentence, :language, :agent_token, :verbose, :now
      )
    end

    def build_log_request
      parameters = interpret_parameters
      context = {}
      request.headers.each do |key, value|
       if key.starts_with?('HTTP_context_')
        context_key = key.slice('HTTP_context_'.length..-1)
        context[context_key] = value
       end
      end
      @log = InterpretRequestLog.new(
        timestamp: Time.now.iso8601(3),
        sentence: parameters['sentence'],
        language: parameters['language'],
        now: parameters['now'],
        agent: @agent,
        context: context
      )
    end
end
