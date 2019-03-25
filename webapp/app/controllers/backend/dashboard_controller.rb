class Backend::DashboardController < Backend::ApplicationController

  def index
    @top_interpretations_agents = Agent
      .select('agents.id, COUNT(interpretations.id) as count')
      .joins(intents: :interpretations)
      .group(:id)
      .order('COUNT(interpretations.id) DESC, agents.agentname')
      .page(params[:interpretations_page]).per(10)

    @top_entities_agents = Agent
      .select('agents.id, COUNT(entities.id) as count')
      .joins(entities_lists: :entities)
      .group(:id)
      .order('COUNT(entities.id) DESC, agents.agentname')
      .page(params[:entities_page]).per(10)

    @top_predecessors_agents = Agent
      .select('agents.id, COUNT(agent_arcs.id) as count')
      .joins(:predecessors)
      .group(:id)
      .order('COUNT(agent_arcs.id) DESC, agents.agentname')
      .page(params[:predecessors_page]).per(10)

    @top_fail_tests_agents = Agent
      .select('agents.id, COUNT(agent_regression_checks.id) as count')
      .joins(:agent_regression_checks)
      .group(:id)
      .where(agent_regression_checks: { state: [:failure, :error] })
      .order('COUNT(agent_regression_checks.id) DESC, agents.agentname')
      .page(params[:tests_page]).per(10)
  end

end
