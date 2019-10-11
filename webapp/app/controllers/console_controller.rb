class ConsoleController < ApplicationController
  before_action :set_owner_and_agent

  def interpret
    access_denied unless current_user.can? :show, @agent

    sentence      = interpret_params[:sentence]
    verbose       = interpret_params[:verbose]
    language      = interpret_params[:language]
    spellchecking = interpret_params[:spellchecking]
    current_tab   = interpret_params[:current_tab]
    now           = interpret_params[:now]

    @regression_check = @agent.find_regression_check_with(sentence, language, spellchecking, now)
    if @regression_check.nil?
      @regression_check = AgentRegressionCheck.new(
        sentence: sentence,
        language: language,
        spellchecking: spellchecking,
        now: now,
        agent: @agent
      )
    end

    nlp = Nlp::Interpret.new(
      agents: [@agent],
      format: 'json',
      sentence: sentence,
      language: language,
      spellchecking: spellchecking,
      verbose: verbose,
      now: now,
      context: {
        user_id: current_user.id,
        client_type: 'console'
      }
    )
    body, status = nlp.proceed

    # Mimic interpret API endpoint behavior
    if status == 200
      console_output = render_to_string(partial: 'api/v1/nlp/interpret', format: 'json', locals: { body: body })
      body = JSON.parse(console_output)
    end

    respond_to do |format|
      format.js {
        @tabs = render_to_string(
          partial: 'tabs',
          locals: { current_tab: current_tab }
        )
        @output = render_to_string(
          partial: 'output',
          locals: {
            status: status,
            body: body,
            agent: @agent,
            current_tab: current_tab,
            endpoint_path: endpoint_path(nlp),
            package_path: full_export_user_agent_url(@agent.owner, @agent, format: 'json')
          }
        )
      }
    end
  end


  private

    def set_owner_and_agent
      @owner = User.friendly.find(params[:user_id])
      @agent = Agent.owned_by(@owner).friendly.find(params[:id])
    end

    def interpret_params
      params.require(:interpret).permit(
        :sentence, :verbose, :language, :spellchecking, :current_tab, :now
      )
    end

    def endpoint_path(nlp)
      interpret_path = "/api/v1/agents/#{@owner.username}/#{@agent.agentname}/interpret.json"
      base_url       = ENV.fetch('VIKYAPP_PUBLIC_URL') { 'http://localhost:3000' }
      parameters = {
        agent_token: @agent.api_token,
        sentence: nlp.sentence,
        language: nlp.language,
        spellchecking: nlp.spellchecking,
        verbose: nlp.verbose,
        now: nlp.now
      }
      base_url + interpret_path + '?' + parameters.to_query
    end

end
