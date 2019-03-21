class EntitiesImport

  MAX_FILE_SIZE = 2.megabytes

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
    unless ['text/csv', 'application/vnd.ms-excel'].include? @mime_type
      @errors[:file] << I18n.t('errors.entity.import.wrong_format')
    end
  end

  def valid?
    validate
    @errors[:file].empty?
  end

  def proceed(entities_list)
    return false unless valid?
    options = {
      headers: [
        I18n.t('activerecord.attributes.entity.terms'),
        I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
        I18n.t('activerecord.attributes.entity.solution')
      ],
      skip_blanks: true,
      encoding: 'UTF-8'
    }
    result = true
    entities_array = []
    csv = CSV.new(@file, options)
    entities_list.entities.delete_all if @mode == :replace
    entities_max_position = entities_list.entities.count.zero? ? 0 : entities_list.entities.maximum(:position)
    begin
      header_valid?(csv)
      line_count = count_lines(csv)
      csv.each_with_index do |row, index|
        next if index.zero?
        row_length_valid?(row, csv.lineno - 1)
        auto_solution = parse_auto_solution(row)
        solution = auto_solution ? '' : parse_solution(row)
        entity = Entity.new(
          terms: parse_terms(row),
          auto_solution_enabled: auto_solution,
          solution: solution,
          position: entities_max_position + line_count - @count,
          entities_list: entities_list
        )
        entity.validate!
        entities_array << entity
        @count += 1
      end
      Entity.import entities_array, validate: false
      entities_array.each do |entity|
        entity.run_callbacks(:save) { true }
      end
    rescue ActiveRecord::ActiveRecordError => e
      @errors[:file] << "#{e.message} in line #{csv.lineno - 1}"
      result = false
    rescue CSV::MalformedCSVError => e
      @errors[:file] << "Bad CSV format: #{e.message}"
      result = false
    end
    result
  end


  private

    def count_lines(csv)
      line_count = 0
      csv.each { line_count += 1 }
      line_count -= 1
      csv.rewind
      line_count
    end

    def header_valid?(csv)
      header_row = csv.shift
      if header_row['Terms'].downcase != 'terms' || header_row['Auto solution'].downcase != 'auto solution' || header_row['Solution'].downcase != 'solution'
        raise CSV::MalformedCSVError, I18n.t('errors.entity.import.missing_header')
      end
      csv.rewind
    end

    def row_length_valid?(row, row_number)
      return if row['Terms'].present? && row['Auto solution'].present? && row['Solution'].present?
      if row['Terms'].nil? || row['Auto solution'].nil?
        raise CSV::MalformedCSVError, I18n.t('errors.entity.import.missing_column', row_number: row_number)
      end
      if !parse_auto_solution(row) && row['Solution'].blank?
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
      row['Solution'].nil? ? '' : row['Solution']
    end
end
