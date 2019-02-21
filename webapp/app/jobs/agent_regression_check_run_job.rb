class AgentRegressionCheckRunJob < ApplicationJob
  queue_as :agent_regression_checks

  def perform(agent)
    agent.agent_regression_checks.update_all(state: 'running')
    notify(agent)
    agent.agent_regression_checks.order("RANDOM()").each do |test|
      test.run
      notify(agent)
    end
  end

  private

    def notify(agent)
      ActionCable.server.broadcast(
        "agent_regression_checks_channel_#{agent.id}",
        agent_id: agent.id,
        timestamp: Time.now.to_f * 1000,
        payload: JSON.parse(agent.regression_checks_to_json)
      )
    end
end
