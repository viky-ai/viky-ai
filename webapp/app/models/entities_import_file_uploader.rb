# frozen_string_literal: true

class EntitiesImportFileUploader < Shrine
  
  plugin :activerecord
  plugin :validation_helpers
  
  # csv file is detected as text/plain
  plugin :determine_mime_type, analyzer: ->(io, analyzers) do
    mime_type = analyzers[:file].call(io)
    mime_type = analyzers[:mime_types].call(io) if mime_type == "text/plain"
    mime_type
  end
  
  plugin :delete_raw
  plugin :pretty_location
  
  Attacher.validate do
    validate_mime_type_inclusion %w[text/csv application/vnd.ms-excel], message: I18n.t('errors.entity.import.wrong_format')
    validate_extension_inclusion %w[csv xls], message: I18n.t('errors.entity.import.wrong_extension')
  end

end
