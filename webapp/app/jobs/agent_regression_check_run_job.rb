class AgentRegressionCheckRunJob < ApplicationJob
  queue_as :agent_regression_checks

  def perform(agent_test)
    ActionCable.server.broadcast 'agent_regression_checks_channel',
                                 agent_id: agent_test.agent.id,
                                 check_id: agent_test.id,
                                 state: 'running'
    agent_test.run
    ActionCable.server.broadcast 'agent_regression_checks_channel',
                                 agent_id: agent_test.agent.id,
                                 check_id: agent_test.id,
                                 state: agent_test.state
  end
end
