class EntitiesImport
  attr_reader :filename
  attr_reader :errors

  def initialize(params = {})
    if params.present?
      @filename = params[:file].original_filename
      @mime_type = params[:file].content_type
    end
    @errors = {
      file: []
    }
  end

  def validate
    @errors[:file] << I18n.t('errors.entity.import.no_file') unless @filename.present?
    @errors[:file] << I18n.t('errors.entity.import.wrong_format') unless @mime_type == 'text/csv'
  end

  def valid?
    validate
    @errors[:file].empty?
  end
end
