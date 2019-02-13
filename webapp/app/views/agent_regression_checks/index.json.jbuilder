if @agent.agent_regression_checks.exists?
  visible = true
  status = @agent.regression_checks_global_state
  count  = @agent.agent_regression_checks.count
  failed_count = @agent.agent_regression_checks.failure.count
  last_update = @agent.agent_regression_checks.order('updated_at').last.updated_at
  tests = @agent.agent_regression_checks
else
  visible = false
  status  = 'unknown'
  count   = 0
  failed_count = 0
  last_update = Time.now
  tests = []
end

json.timestamp Time.now.to_f * 1000

json.summary do
  json.visible visible
  json.status status
  json.count count
  json.count_i18n t('views.console.regression_check.tests', count: count)
  json.failed_count failed_count
  json.failed_count_i18n t('views.console.regression_check.failures', count: failed_count)
  json.last_update l(last_update, format: :short)
end

json.interpret_url interpret_user_agent_path(@agent.owner, @agent)

json.tests tests do |test|
  json.partial! 'agent_regression_checks/regression_check', test: test
end
