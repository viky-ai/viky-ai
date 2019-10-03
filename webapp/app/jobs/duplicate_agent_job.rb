class DuplicateAgentJob < ApplicationJob
  queue_as :default

  discard_on ActiveJob::DeserializationError

  retry_on ActiveRecord::Deadlocked

  def perform(agent, current_user)
    duplicator = AgentDuplicator.new(agent, current_user)

    if duplicator.respects_quota?
      new_agent = duplicator.duplicate
      if new_agent.errors.any?
        UserNotificationsChannel.broadcast_to current_user,
          alert: I18n.t(
            'views.agents.duplication.failed_message',
            errors: new_agent.errors.full_messages.join(', ')
          )
      else
        UserNotificationsChannel.broadcast_to current_user,
          notice: I18n.t('views.agents.duplication.success_message', name: new_agent.name)
      end
    else
      UserNotificationsChannel.broadcast_to current_user,
        alert: I18n.t('views.agents.duplication.quota_exceeded_message')
    end
  end
end
