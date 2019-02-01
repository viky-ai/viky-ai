if @agent.agent_regression_checks.exists?
  visible = true
  status = @agent.any_tests_failed? ? 'failure' : 'success'
  count  = @agent.agent_regression_checks.count
  failed_count = @agent.agent_regression_checks.failed.count
  last_update = @agent.agent_regression_checks.order('updated_at').last.updated_at
  tests = @agent.agent_regression_checks
else
  visible = false
  status  = 'running'
  count   = 0
  failed_count = 0
  last_update = Time.now
  tests = []
end

json.summary do
  json.visible visible
  json.status status
  json.count count
  json.count_i18n t('views.console.regression_check.tests', count: count)
  json.failed_count failed_count
  json.failed_count_i18n t('views.console.regression_check.failures', count: failed_count)
  json.last_update l(last_update, format: :short)
end

json.tests tests do |test|
  json.state test.state
  json.sentence test.sentence
  json.language test.language
  json.expected test.expected
  json.got test.got
end
