class EntitiesImport < ApplicationRecord

  BATCH_SIZE = 1000

  include EntitiesImportFileUploader::Attachment.new(:file)
  validates_presence_of :file, message: I18n.t('errors.entities_import.no_file')

  belongs_to :entities_list

  enum mode: [:append, :replace]

  validates :mode, presence: true

  def proceed
    options = {
      headers: [
        I18n.t('activerecord.attributes.entity.terms'),
        I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
        I18n.t('activerecord.attributes.entity.solution')
      ],
      skip_blanks: true,
      encoding: 'UTF-8'
    }
    count = 0
    columns = [:terms, :auto_solution_enabled, :solution, :position, :entities_list_id]
    entities_array = []
    ActiveRecord::Base.transaction do
      entities_list.entities.delete_all if mode == 'replace'
      entities_max_position = entities_list.entities.count.zero? ? 0 : entities_list.entities.maximum(:position)
      begin
        line_count = count_lines(options)
        index = 0
        CSV.foreach(get_file_location, options) do |row|
          if index.zero?
            header_valid?(row)
            index += 1
            next
          end
          row_length_valid?(row, index)
          auto_solution = parse_auto_solution(row)
          terms = parse_terms(row)
          solution = parse_solution(row, terms, auto_solution)
          position = entities_max_position + line_count - count
          entities_array << [terms, auto_solution, solution, position, entities_list.id]
          if (index % BATCH_SIZE).zero?
            Rails.logger.silence(Logger::INFO) do
              Entity.import! columns, entities_array
            end
            entities_array = []
          end
          count += 1
          index += 1
        end
        Rails.logger.silence(Logger::INFO) do
          Entity.import! columns, entities_array unless entities_array.empty?
          update_entities_list
        end
      rescue ActiveRecord::ActiveRecordError => e
        errors[:file] << "#{e.message}"
        count = 0
        raise ActiveRecord::Rollback
      rescue CSV::MalformedCSVError => e
        errors[:file] << "Bad CSV format: #{e.message}"
        count = 0
        raise ActiveRecord::Rollback
      end
    end
    count
  end

  private

    def count_lines(options)
      line_count = 0
      CSV.foreach(get_file_location, options) do |row|
        line_count += 1
      end
      line_count -= 1
      line_count
    end

    def header_valid?(header_row)
      if header_row['Terms'].downcase != 'terms' || header_row['Auto solution'].downcase != 'auto solution' || header_row['Solution'].downcase != 'solution'
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_header')
      end
    end

    def row_length_valid?(row, row_number)
      return if row['Terms'].present? && row['Auto solution'].present? && row['Solution'].present?
      if row['Terms'].nil? || row['Auto solution'].nil?
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_column', row_number: row_number)
      end
      if !parse_auto_solution(row) && row['Solution'].blank?
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_column', row_number: row_number)
      end
    end

    def parse_terms(row)
      terms = row['Terms']
      if terms.present?
        terms = terms.tr('|', "\n")
        terms = EntityTermsParser.new(terms).proceed
      end
      terms
    end

    def parse_auto_solution(row)
      if row['Auto solution'].blank?
        raise ActiveRecord::ActiveRecordError, I18n.t('errors.entities_import.unexpected_autosolution')
      end
      case row['Auto solution'].downcase
        when 'true'
          true
        when 'false'
          false
        else
          raise ActiveRecord::ActiveRecordError, I18n.t('errors.entities_import.unexpected_autosolution')
      end
    end

    def parse_solution(row, terms, auto_solution)
      solution = row['Solution']
      if terms.present? && auto_solution
        if terms.is_a? String
          solution = terms
        else
          solution = terms.first['term']
        end
      end
      solution
    end

    def update_entities_list
      entities_list.agent.update_locales
      entities_list.touch
    end

    def get_file_location
      if file.storage.is_a? Shrine::Storage::FileSystem
        file.storage.path(file.id)
      else # for test environment when Shrine uses in-memory storage
        tempfile = File.open('temp.csv', 'w+')
        file.open do |f|
          tempfile.write(f.read)
        end
        tempfile.close
        tempfile.path
      end
    end

end
