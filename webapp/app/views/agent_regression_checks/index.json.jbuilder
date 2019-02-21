if @agent.agent_regression_checks.exists?
  visible = true
  status = @agent.regression_checks_global_state
  count  = @agent.agent_regression_checks.count
  running_count = @agent.agent_regression_checks.where(state: [:running, :unknown]).count
  success_count = @agent.agent_regression_checks.success.count
  failed_count = @agent.agent_regression_checks.where(state: [:failure, :error]).count
  last_update = @agent.agent_regression_checks.order('updated_at').last.updated_at
else
  visible = false
  status  = 'unknown'
  count   = 0
  running_count = 0
  success_count = 0
  failed_count = 0
  last_update = Time.now
end

json.timestamp Time.now.to_f * 1000

json.summary do
  json.visible visible
  json.status status

  json.count count
  json.count_i18n t('views.console.regression_check.summary.tests', count: count)

  json.running_count running_count
  json.running_count_i18n t('views.console.regression_check.summary.running', count: running_count)

  json.success_count success_count
  json.success_count_i18n t('views.console.regression_check.summary.success', count: success_count)

  json.failed_count failed_count
  json.failed_count_i18n t('views.console.regression_check.summary.failures', count: failed_count)

  json.last_update l(last_update, format: :short)
end

owner = @agent.owner

json.interpret_url interpret_user_agent_path(owner, @agent)
json.update_positions_url update_positions_user_agent_agent_regression_checks_path(owner, @agent)

json.tests @agent.agent_regression_checks.order(position: :desc),
  partial: 'agent_regression_checks/regression_check',
  as: :test,
  locals: {
    agent: @agent,
    owner: owner
  }
