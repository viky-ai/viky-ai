class Backend::DashboardController < Backend::ApplicationController

  def index
    @top_interpretations_agents = Agent
      .select('agents.id, COUNT(interpretations.id) as count')
      .joins(intents: :interpretations)
      .group(:id)
      .order('count DESC, agents.name')
      .page(params[:interpretations_page]).per(10)

    @top_entities_agents = Agent
      .select('agents.id, COUNT(entities.id) as count')
      .joins(entities_lists: :entities)
      .group(:id)
      .order('count DESC, agents.name')
      .page(params[:entities_page]).per(10)

    @top_predecessors_agents = Agent
      .select('agents.id, COUNT(agent_arcs.id) as count')
      .joins(:predecessors)
      .group(:id)
      .order('count DESC, agents.name')
      .page(params[:predecessors_page]).per(10)

    @top_fail_tests_agents = Agent
      .select('agents.id, COUNT(agent_regression_checks.id) as count')
      .joins(:agent_regression_checks)
      .group(:id)
      .where(agent_regression_checks: { state: [:failure, :error] })
      .order('count DESC, agents.name')
      .page(params[:tests_page]).per(10)

    @top_expressions_users = Kaminari
      .paginate_array(User.expressions_count)
      .page(params[:expressions_page]).per(10)

    to = DateTime.now
    from = DateTime.now - 30.days

    @top_api_requests_under_quota = Kaminari
      .paginate_array(InterpretRequestLog.requests_per_owners(from, to, [200, 500, 422]))
      .page(params[:requests_page]).per(10)

    if Feature.quota_enabled?
      @top_api_requests_over_quota = Kaminari
        .paginate_array(InterpretRequestLog.requests_per_owners(from, to, [503]))
        .page(params[:requests_page]).per(10)
    end

  end
end
