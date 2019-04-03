require_relative 'measure_helper.rb'

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
    entities_array = []
    print_memory_usage do
      print_time_spent do
        ActiveRecord::Base.transaction do
          entities_list.entities.delete_all if mode == 'replace'
          entities_max_position = entities_list.entities.count.zero? ? 0 : entities_list.entities.maximum(:position)
          begin
            line_count = count_lines(options)
            index = 0
            file.download do |tempfile|
              tempfile = tempfile.set_encoding('UTF-8')
              CSV.foreach(tempfile.path, options) do |row|
                if index.zero?
                  header_valid?(row)
                  index += 1
                  next
                end
                row_length_valid?(row, index)
                auto_solution = parse_auto_solution(row)
                solution = auto_solution ? '' : parse_solution(row)
                entity = Entity.new(
                  terms: parse_terms(row),
                  auto_solution_enabled: auto_solution,
                  solution: solution,
                  position: entities_max_position + line_count - count,
                  entities_list: entities_list
                )
                entity.validate!
                entities_array << entity
                if (index % BATCH_SIZE).zero?
                  Entity.import entities_array, validate: false
                  entities_array = []
                end
                count += 1
                index += 1
              end
            end
            Entity.import entities_array, validate: false unless entities_array.empty?
            update_entities_list
          rescue ActiveRecord::ActiveRecordError => e
            errors[:file] << "#{e.message} in line #{index}"
            count = 0
            raise ActiveRecord::Rollback
          rescue CSV::MalformedCSVError => e
            errors[:file] << "Bad CSV format: #{e.message}"
            count = 0
            raise ActiveRecord::Rollback
          end
        end
      end
    end
    count
  end
  
  private

    def count_lines(options)
      line_count = 0
      file.download do |tempfile|
        CSV.foreach(tempfile.path, options) do |row|
          line_count += 1
        end
      end
      line_count -= 1
      ap line_count
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
      row['Terms'].present? ? row['Terms'].tr('|', "\n") : row['Terms']
    end

    def parse_auto_solution(row)
      raise ActiveRecord::ActiveRecordError, I18n.t('errors.entities_import.unexpected_autosolution') if row['Auto solution'].blank?
      case row['Auto solution'].downcase
        when 'true'
          true
        when 'false'
          false
        else
          raise ActiveRecord::ActiveRecordError, I18n.t('errors.entities_import.unexpected_autosolution')
      end
    end

    def parse_solution(row)
      row['Solution'].nil? ? '' : row['Solution']
    end

    def update_entities_list
      entities_list.agent.update_locales
      entities_list.touch
    end

end
