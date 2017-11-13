class ConsoleController < ApplicationController

  def interpret
    owner = User.friendly.find(params[:user_id])
    agent = owner.agents.friendly.find(params[:id])

    access_denied unless current_user.can? :show, agent

    sentence = interpret_params[:sentence]
    data = get_interpretation(owner, agent, sentence)
    current_tab = interpret_params[:current_tab]

    respond_to do |format|
      format.js {
        @tabs = render_to_string(
          partial: 'tabs',
          locals: { current_tab: current_tab }
        )
        @output = render_to_string(
          partial: 'output',
          locals: { data: data, current_tab: current_tab }
        )
      }
    end
  end


  private

    def interpret_params
      params.require(:interpret).permit(
        :sentence, :current_tab
      )
    end

    def get_interpretation(owner, agent, sentence)
      req = Rack::Request.new({ "rack.input" => {}, "REQUEST_METHOD" => "GET" })
      req.path_info = "/api/v1/agents/#{owner.username}/#{agent.agentname}/interpret.json"
      req.update_param(:agent_token, agent.api_token)
      req.update_param(:sentence, sentence)
      response = Rails.application.call(req.env)
      status, headers, body = response
      {
        status: status,
        body: body.first
      }
    end

end
