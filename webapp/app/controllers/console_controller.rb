class ConsoleController < ApplicationController

  def interpret
    owner = User.friendly.find(params[:user_id])
    agent = owner.agents.friendly.find(params[:id])

    access_denied unless current_user.can? :show, agent

    sentence    = interpret_params[:sentence]
    verbose     = interpret_params[:verbose]
    language    = interpret_params[:language]
    current_tab = interpret_params[:current_tab]
    now         = interpret_params[:now]

    data = get_interpretation(agent, sentence, verbose, language, now)

    @regression_check =  agent.find_regression_check_with(sentence, language, now)
    if @regression_check.nil?
      @regression_check = AgentRegressionCheck.new(
        sentence: sentence,
        language: language,
        now: now,
        agent: agent
      )
    end

    respond_to do |format|
      format.js {
        @tabs = render_to_string(
          partial: 'tabs',
          locals: { current_tab: current_tab }
        )
        user_query = {
          sentence: sentence,
          language: language,
          now: now,
          verbose: verbose != 'false'
        }
        @output = render_to_string(
          partial: 'output',
          locals: { data: data, current_tab: current_tab, agent: agent, user_query: user_query }
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

    def get_interpretation(agent, sentence, verbose, language, now)
      request_params = {
        sentence: sentence,
        language: language,
        now: now,
        verbose: verbose,
        client_type: 'console',
        user_id: current_user.id,
      }
      result = Nlp::PublicInterpret.request_public_api(request_params, agent)
      result[:package_path] = full_export_user_agent_url(agent.owner, agent, format: 'json') if current_user.admin?
      result
    end
end
