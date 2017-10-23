class Api::V1::NlsController < Api::V1::ApplicationController
  before_action :set_agent
  before_action :authenticate_agent_access!

  def interpret
    @nls = Nls::Interpret.new(params)
    @nls.interpret

    unless @nls.errors.empty?
      render json: {message: @nls.errors.messages}, status: 422 and return
    end
  end


  private

    def authenticate_agent_access!
      agent_token = request.headers["Agent-Token"] || params[:agent_token]
      language = request.headers["Accept-Language"] || params[:language]

      if @agent.nil? || @agent.api_token != agent_token
        render json: {message: t('controllers.api.v1.nls.interpret.wrong_agent_token', agent_name: params[:id])}, status: 401 and return
      end
      params.merge({language: language})
    end

    def set_agent
      user = User.friendly.find(params[:user_id])
      @agent = user.agents.friendly.find(params[:id])
    end
end
