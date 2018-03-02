class EntitiesImport

  require 'csv'

  MAX_FILE_SIZE = 2*1024*1024

  attr_reader :file
  attr_reader :errors
  attr_reader :count

  def initialize(params = {})
    @errors = {
      file: []
    }
    @mode = :append
    @count = 0
    if params.present? && params[:file].present?
      @file      = params[:file].tempfile
      @mime_type = params[:file].content_type
      @mode      = :replace if params[:mode] == 'replace' || params[:mode] == :replace
      @file.set_encoding('utf-8')
    end
  end

  def validate
    if @file.nil?
      @errors[:file] << I18n.t('errors.entity.import.no_file')
      return
    end
    @errors[:file] << I18n.t('errors.entity.import.max_file_size') unless @file.size < MAX_FILE_SIZE
    @errors[:file] << I18n.t('errors.entity.import.wrong_format') unless @mime_type == 'text/csv'
  end

  def valid?
    validate
    @errors[:file].empty?
  end

  def proceed(entities_list)
    return false unless valid?
    options = {
      col_sep: ',',
      quote_char: "'",
      force_quotes: true,
      headers: [I18n.t('activerecord.attributes.entity.terms'), I18n.t('activerecord.attributes.entity.auto_solution_enabled'), I18n.t('activerecord.attributes.entity.solution')],
      skip_blanks: true,
      encoding: 'UTF-8'
    }
    result = true
    ActiveRecord::Base.transaction do
      csv = CSV.new(@file, options)
      entities_list.entities.delete_all if @mode == :replace
      begin
        check_header(csv.shift)
        csv.rewind
        csv.each_with_index do |row, index|
          next if index.zero?
          check_row_length(row, csv.lineno - 1)
          Entity.create!(
            terms:                 parse_terms(row),
            auto_solution_enabled: parse_auto_solution(row),
            solution:              parse_solution(row),
            entities_list:         entities_list
          )
          @count += 1
        end
      rescue ActiveRecord::ActiveRecordError => e
        @errors[:file] << "#{e.message} in line #{csv.lineno - 1}"
        result = false
        raise ActiveRecord::Rollback
      rescue CSV::MalformedCSVError => e
        @errors[:file] << e.message
        result = false
        raise ActiveRecord::Rollback
      end
    end
    result
  end


  private

    def check_header(header_row)
      if header_row['Terms'].downcase != 'terms' || header_row['Auto solution'].downcase != 'auto solution' || header_row['Solution'].downcase != 'solution'
        raise CSV::MalformedCSVError, I18n.t('errors.entity.import.missing_header')
      end
    end

    def check_row_length(row, row_number)
      if row['Terms'].nil? || row['Auto solution'].nil? || row['Solution'].nil?
        raise CSV::MalformedCSVError, I18n.t('errors.entity.import.missing_column', row_number: row_number)
      end
    end

    def parse_terms(row)
      row['Terms'].present? ? row['Terms'].tr('|', "\n") : row['Terms']
    end

    def parse_auto_solution(row)
      raise ActiveRecord::ActiveRecordError, I18n.t('errors.entity.import.unexpected_autosolution') if row['Auto solution'].blank?
      case row['Auto solution'].downcase
        when 'true'
          true
        when 'false'
          false
        else
          raise ActiveRecord::ActiveRecordError, I18n.t('errors.entity.import.unexpected_autosolution')
      end
    end

    def parse_solution(row)
      row['Solution']
    end
end
