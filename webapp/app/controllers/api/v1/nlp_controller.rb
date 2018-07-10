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
          @log.with_response(@response[:status])
          unless @response[:status] == '200'
            render json: @response[:body], status: @response[:status]
          end
        else
          @log.with_response(422)
          render json: { errors: @nlp.errors.full_messages }, status: 422
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
        @log.with_response(401).save
        render json: { errors: [t('controllers.api.access_denied')] }, status: 401
      end
    end

    def interpret_parameters
      params.permit(
        :ownername, :agentname, :format, :sentence, :language, :agent_token, :verbose, :now
      )
    end

    def build_log_request
      parameters = interpret_parameters
      @log = InterpretRequestLog.new(
        timestamp: Time.now.iso8601(3),
        sentence: parameters['sentence'],
        language: parameters['language'],
        now: parameters['now'],
        agent: @agent
      )
    end
end
