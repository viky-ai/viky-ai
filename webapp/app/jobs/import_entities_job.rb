class ImportEntitiesJob < ApplicationJob
  queue_as :import_entities

  def perform(entities_import)
    # TODO how to notify user??
    if entities_import.proceed
      # TODO count
      UserNotificationsChannel.broadcast_to current_user,
        notice: I18n.t('views.entities_lists.show.import.select_import.success')
    else
      Rails.logger.info 'Failed'
    end
  end
end
