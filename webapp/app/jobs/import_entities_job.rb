class ImportEntitiesJob < ApplicationJob
  queue_as :import_entities

  def perform(entities_import, current_user)
    count = entities_import.proceed
    if count.zero?
      UserNotificationsChannel.broadcast_to current_user,
                                            alert: I18n.t('views.entities_lists.show.import.select_import.failed',
                                                          errors: entities_import.errors.full_messages.join(', '))
    else
      UserNotificationsChannel.broadcast_to current_user,
                                            notice: I18n.t('views.entities_lists.show.import.select_import.success',
                                                           count: count)
    end
    entities_import.destroy
  end
end
