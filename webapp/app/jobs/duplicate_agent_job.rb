require_relative 'measure_helper.rb'
class DuplicateAgentJob < ApplicationJob
  queue_as :default

  discard_on ActiveJob::DeserializationError

  retry_on ActiveRecord::Deadlocked

  def perform(agent, current_user)
    print_memory_usage do
      print_time_spent do
        new_agent = AgentDuplicator.new(agent, current_user).duplicate
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
      end
    end
  end
end
