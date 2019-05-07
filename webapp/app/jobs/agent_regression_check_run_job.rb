class AgentRegressionCheckRunJob < ApplicationJob
  queue_as :agent_regression_checks

  def perform(agent)
    now = Time.zone.now
    agent.agent_regression_checks.update_all(state: 'running', updated_at: now)
    notify(agent)

    count = agent.agent_regression_checks.count

    agent.agent_regression_checks.order("RANDOM()").each_with_index do |test, i|
      next if test.reload.updated_at > now
      test.run
      if count > 10
        notify(agent) if ((i + 1) % (count / 10)) == 0
      else
        notify(agent)
      end
    end

    notify(agent)
  end

  private

    def notify(agent)
      ActionCable.server.broadcast(
        "agent_console_channel_#{agent.id}",
        agent_id: agent.id,
        timestamp: Time.now.to_f * 1000,
        payload: JSON.parse(agent.regression_checks_to_json)
      )
    end
end
