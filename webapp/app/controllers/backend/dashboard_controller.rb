class Backend::DashboardController < Backend::ApplicationController

  def index
    @top_formulations_agents = Agent
      .select('agents.id, COUNT(formulations.id) as count')
      .joins(intents: :formulations)
      .group(:id)
      .order('count DESC, agents.name')
      .page(params[:formulations_page]).per(10)

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

    @top_expressions_per_owners = User
      .expressions_count_per_owners
      .page(params[:expressions_page]).per(10)

    to = DateTime.now
    from = DateTime.now - 30.days

    @top_api_requests_under_quota = Kaminari
      .paginate_array(InterpretRequestReporter.new.under_quota.between(from, to).count_per_owner)
      .page(params[:requests_page]).per(10)

    if Feature.quota_enabled?
      @top_api_requests_over_quota = Kaminari
        .paginate_array(InterpretRequestReporter.new.over_quota.between(from, to).count_per_owner)
        .page(params[:requests_page]).per(10)
    end

  end
end
