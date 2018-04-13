class DuplicateAgentJob < ApplicationJob
  queue_as :default

  discard_on ActiveJob::DeserializationError

  retry_on ActiveRecord::Deadlocked

  def perform(*args)
    agent = args[0]
    current_user = args[1]
    new_agent = AgentDuplicator.new(agent, current_user).duplicate
    if new_agent.valid?
      new_agent.save!
      UserNotificationsChannel.broadcast_to current_user, notice: I18n.t('views.agents.duplication.success_message', name: new_agent.name)
    else
      UserNotificationsChannel.broadcast_to current_user, alert: I18n.t('views.agents.duplication.failed_message',
                                                                        errors: new_agent.errors.full_messages.join(', '))
    end
  end
end
