class AgentRegressionCheckRunJob < ApplicationJob
  queue_as :agent_regression_checks

  def perform(agent_test, user_who_started_run, base_url)
    agent_test.run user_who_started_run, base_url
  end
end
