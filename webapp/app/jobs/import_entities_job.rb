class ImportEntitiesJob < ApplicationJob
  queue_as :import_entities

  def perform(entities_import, current_user)
    sleep(2)
    count = entities_import.proceed
    agent = entities_import.entities_list.agent

    if count.zero? && entities_import.errors.any?
      errors = entities_import.errors.full_messages
      entities_import.status = :failure
      entities_import.duration = DateTime.now.to_time - entities_import.created_at.to_time
      entities_import.save!

      html = ApplicationController.render(
        partial: 'entities/import_failure',
        locals: { errors: errors }
      )

      UserNotificationsChannel.broadcast_to current_user,
        trigger: {
          event: "entities_import:failure",
          data: { html: html }
        }
    else
      entities_import.status = :success
      entities_import.duration = DateTime.now.to_time - entities_import.created_at.to_time
      entities_import.save!

      html = ApplicationController.render(
        partial: 'entities/import_success',
        locals: { count: count }
      )

      UserNotificationsChannel.broadcast_to current_user,
        trigger: {
          event: "entities_import:success",
          data: { html: html }
        }
    end

    entities_import.destroy
  end
end
