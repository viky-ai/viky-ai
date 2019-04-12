class EntitiesImportFileUploader < Shrine

  MAX_FILE_SIZE = 500.megabytes

  plugin :activerecord
  plugin :validation_helpers

  # csv file is detected as text/plain
  plugin :determine_mime_type, analyzer: ->(io, analyzers) do
    mime_type = analyzers[:file].call(io)
    mime_type = analyzers[:mime_types].call(io) if mime_type == "text/plain"
    mime_type
  end

  plugin :delete_promoted
  plugin :pretty_location

  Attacher.validate do
    validate_max_size MAX_FILE_SIZE,
      message: I18n.t(
        'errors.entities_import.max_file_size',
        limit: ActiveSupport::NumberHelper.number_to_human_size(MAX_FILE_SIZE)
      )
    validate_mime_type_inclusion %w[text/csv application/vnd.ms-excel],
      message: I18n.t('errors.entities_import.wrong_format')
    validate_extension_inclusion %w[csv xls],
      message: I18n.t('errors.entities_import.wrong_extension')
  end

end
