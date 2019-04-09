class ImportEntitiesJob < ApplicationJob
  queue_as :import_entities

  def perform(entities_import, current_user)
    entities_list = entities_import.entities_list
    agent         = entities_list.agent

    notify_start(agent.id, entities_list.id)

    count = entities_import.proceed

    if count.zero? && entities_import.errors.any?
      errors = entities_import.errors.full_messages
      entities_import.status = :failure
      entities_import.duration = DateTime.now.to_time - entities_import.created_at.to_time
      entities_import.save!
      notify_failure(errors, agent.id, entities_list.id, current_user.id)
    else
      entities_import.status = :success
      entities_import.duration = DateTime.now.to_time - entities_import.created_at.to_time
      entities_import.save!
      notify_success(count, agent.id, entities_list.id)
    end

    entities_import.destroy
  end

  def notify_start(agent_id, entities_list_id)
    html = ApplicationController.render(
      partial: 'entities/import_running'
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{agent_id}",
      trigger: {
        event: "entities_import:start",
        data: {
          entities_list_id: entities_list_id,
          html: html
        }
      }
    )
  end

  def notify_failure(errors, agent_id, entities_list_id, current_user_id)
    html = ApplicationController.render(
      partial: 'entities/import_failure',
      locals: { errors: errors }
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{agent_id}",
      trigger: {
        event: "entities_import:failure",
        data: {
          entities_list_id: entities_list_id,
          current_user_id: current_user_id,
          html: html
        }
      }
    )
  end

  def notify_success(count, agent_id, entities_list_id)
    html = ApplicationController.render(
      partial: 'entities/import_success',
      locals: { count: count }
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{agent_id}",
      trigger: {
        event: "entities_import:success",
        data: {
          entities_list_id: entities_list_id,
          html: html
        }
      }
    )
  end
end
