class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :authenticate_agent_access!

  # GET /agents/:user_id/:id/interpret.json
  def interpret
    @nlp = Nlp::Interpret.new(interpret_parameters)

    if @nlp.valid?
      @nlp.interpret
    else
      render json: {message: @nlp.errors.messages}, status: 422 and return
    end
  end


  private

    def authenticate_agent_access!
      user = User.friendly.find(params[:user_id])
      agent = user.agents.friendly.find(params[:id])

      agent_token = request.headers["Agent-Token"] || params[:agent_token]
      language = request.headers["Accept-Language"] || params[:language]

      if agent.nil? || agent.api_token != agent_token
        render json: {message: t('controllers.api.v1.nls.interpret.wrong_agent_token', agent_name: params[:id])}, status: 401 and return
      end
      params.merge({language: language})
    end

    def interpret_parameters
      params.permit(
        :user_id, :id, :format, :sentence, :language, :agent_token
      )
    end

end
