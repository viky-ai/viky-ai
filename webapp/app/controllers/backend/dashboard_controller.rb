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

    to = DateTime.now
    from = DateTime.now - 30.days
    @top_requests_users = paginate_requests(InterpretRequestLog.requests_over_users(from, to), 10, params[:requests_page])
  end

  private
  def paginate_requests(requests_array, per_page, page)
    page ||= 1
    start_index = per_page * (page.to_i - 1)
    end_index = start_index + (per_page -1) 
    end_index = end_index >= requests_array.size ? -1 : end_index
    Kaminari.paginate_array(requests_array[start_index..end_index], total_count: requests_array.size).page(page.to_i).per(per_page)
  end

end
