class AgentRegressionCheckRunJob < ApplicationJob
  queue_as :agent_regression_checks

  def perform(agent)
    ActionCable.server.broadcast 'agent_regression_checks_channel',
                                 agent_id: agent.id,
                                 check_id: 'summary',
                                 state: 'running',
                                 last_update: I18n.l(agent.agent_regression_checks.order('updated_at').last.updated_at, format: :short),
                                 failed_count_i18n: I18n.t('views.console.regression_check.failures', count: agent.agent_regression_checks.failure.count),
                                 timestamp: Time.now.to_f * 1000

    agent.agent_regression_checks.each do |test|
      ActionCable.server.broadcast 'agent_regression_checks_channel',
                                   agent_id: agent.id,
                                   check_id: test.id,
                                   state: 'running',
                                   timestamp: Time.now.to_f * 1000
      test.run
      ActionCable.server.broadcast 'agent_regression_checks_channel',
                                   agent_id: agent.id,
                                   check_id: test.id,
                                   state: test.state,
                                   timestamp: Time.now.to_f * 1000
      ActionCable.server.broadcast 'agent_regression_checks_channel',
                                   agent_id: agent.id,
                                   check_id: 'summary',
                                   state: agent.any_tests_failed? ? 'failure' : 'success',
                                   last_update: I18n.l(agent.agent_regression_checks.order('updated_at').last.updated_at, format: :short),
                                   failed_count_i18n: I18n.t('views.console.regression_check.failures', count: agent.agent_regression_checks.failure.count),
                                   timestamp: Time.now.to_f * 1000
    end
  end
end
