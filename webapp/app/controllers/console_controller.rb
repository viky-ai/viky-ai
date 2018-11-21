class ConsoleController < ApplicationController

  CLIENT_TYPE = 'console'

  def interpret
    owner = User.friendly.find(params[:user_id])
    agent = owner.agents.friendly.find(params[:id])

    access_denied unless current_user.can? :show, agent

    sentence    = interpret_params[:sentence]
    verbose     = interpret_params[:verbose]
    language    = interpret_params[:language]
    current_tab = interpret_params[:current_tab]
    now         = interpret_params[:now]

    data = get_interpretation(owner, agent, sentence, verbose, language, now)

    respond_to do |format|
      format.js {
        @tabs = render_to_string(
          partial: 'tabs',
          locals: { current_tab: current_tab }
        )
        @output = render_to_string(
          partial: 'output',
          locals: { data: data, current_tab: current_tab}
        )
      }
    end
  end


  private

    def interpret_params
      params.require(:interpret).permit(
        :sentence, :verbose, :language, :current_tab, :now
      )
    end

    def get_interpretation(owner, agent, sentence, verbose, language, now)

      req = Rack::Request.new({
        "rack.input" => {},
        "REQUEST_METHOD" => "GET",
        "HTTP_context_client_type" => CLIENT_TYPE,
        "HTTP_context_user_id" => current_user.id})
      req.path_info = "/api/v1/agents/#{owner.username}/#{agent.agentname}/interpret.json"

      params = {
        agent_token: agent.api_token,
        sentence: sentence,
        language: language,
        now: now
      }
      params[:verbose] = verbose if verbose == "true"
      params.each { |k, v| req.update_param(k, v) }
      path = request.base_url + req.path_info + '?' + params.to_query

      response = Rails.application.call(req.env)
      status, headers, body = response

      result = {
        path: path,
        status: status,
      }
      result[:body] = status == 200 ? body.first : body.body
      result[:package_path] = full_export_user_agent_url(agent.owner, agent, format: 'json') if current_user.admin?
      result
    end
end
