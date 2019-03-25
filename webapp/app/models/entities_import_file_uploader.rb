# frozen_string_literal: true

class EntitiesImportFileUploader < Shrine
  
  plugin :activerecord
  plugin :validation_helpers
  plugin :delete_raw
  plugin :pretty_location
  
  Attacher.validate do
    validate_mime_type_inclusion %w[text/csv application/vnd.ms-excel], message: I18n.t('errors.entity.import.wrong_format')
    validate_extension_inclusion %w[csv xls], message: I18n.t('errors.entity.import.wrong_format')
  end

end
