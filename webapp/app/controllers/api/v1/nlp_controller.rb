class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :validate_owner_and_agent
  before_action :check_agent_token

  def interpret
    @nlp = Nlp::Interpret.new(interpret_parameters)

    @nlp.agent_token = request.headers["Agent-Token"]     if @nlp.agent_token.blank?
    @nlp.language    = request.headers["Accept-Language"] if @nlp.language.blank?

    respond_to do |format|
      format.json {
        if @nlp.valid?
          @response = @nlp.proceed
          unless @response[:status] == '200'
            render json: @response[:body], status: @response[:status]
          end
        else
          render json: { errors: @nlp.errors.full_messages }, status: 422
        end
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
        render json: { errors: [t('controllers.api.access_denied')] }, status: 401
      end
    end

    def interpret_parameters
      params.permit(
        :ownername, :agentname, :format, :sentence, :language, :agent_token
      )
    end

end
