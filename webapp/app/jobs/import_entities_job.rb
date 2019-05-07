class ImportEntitiesJob < ApplicationJob
  queue_as :import_entities

  def perform(entities_import)
    entities_list = entities_import.entities_list
    agent         = entities_list.agent

    entities_import.filesize = entities_import.file.metadata["size"]
    entities_import.save!

    notify_start(entities_import)

    count = entities_import.proceed

    if count.zero? && entities_import.errors.any?
      errors = entities_import.errors[:file]
      entities_import.status = :failure
      entities_import.save!
      notify_failure(errors, entities_import)
    else
      entities_import.status = :success
      entities_import.duration = DateTime.now.to_time - entities_import.created_at.to_time
      entities_import.save!
      notify_success(count, entities_import)
    end
    entities_import.update(file: nil)
  end

  def notify_start(entities_import)
    html = ApplicationController.render(
      partial: 'entities/import_running',
      locals: { entities_import: entities_import }
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{entities_import.entities_list.agent.id}",
      trigger: {
        event: "entities_import:start",
        data: {
          entities_list_id: entities_import.entities_list.id,
          html: html
        }
      }
    )
  end

  def notify_failure(errors, entities_import)
    html = ApplicationController.render(
      partial: 'entities/import_failure',
      locals: { errors: errors }
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{entities_import.entities_list.agent.id}",
      trigger: {
        event: "entities_import:failure",
        data: {
          entities_list_id: entities_import.entities_list.id,
          import_user_id: entities_import.user.id,
          html: html
        }
      }
    )
  end

  def notify_success(count, entities_import)
    html = ApplicationController.render(
      partial: 'entities/import_success',
      locals: { count: count }
    )
    ActionCable.server.broadcast(
      "agent_interface_channel_#{entities_import.entities_list.agent.id}",
      trigger: {
        event: "entities_import:success",
        data: {
          entities_list_id: entities_import.entities_list.id,
          html: html
        }
      }
    )
  end
end
