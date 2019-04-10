class EntitiesImport < ApplicationRecord
  belongs_to :entities_list
  belongs_to :user

  include EntitiesImportFileUploader::Attachment.new(:file)
  validates_presence_of :file, message: I18n.t('errors.entities_import.no_file'), on: :create
  validate :absence_of_concurrent_import, on: :create
  validates :mode, presence: true

  enum mode: [:append, :replace]
  enum status: [ :running, :success, :failure ]

  def proceed
    count = 0
    entities_list_id = entities_list.id
    columns = [:terms, :auto_solution_enabled, :solution, :position, :entities_list_id]
    entities = []
    ActiveRecord::Base.transaction do
      if mode == 'replace'
        entities_list.entities.delete_all
      end
      if entities_list.entities.empty?
        max_position = 0
      else
        max_position = entities_list.entities.maximum(:position)
      end

      begin
        lines_count = csv_count_lines
        CSV.foreach(csv_file_path, csv_reader_options) do |row|
          if $. == 1
            validate_csv_header!(row)
          else
            validate_csv_row_length!(row, count + 1)
            entities << build_import_line_from_csv_row(
              row,
              max_position + lines_count - count,
              entities_list_id
            )
            if ($. % 1000).zero?
              Rails.logger.silence(Logger::INFO) do
                Entity.import!(columns, entities)
              end
              entities = []
            end
            count += 1
          end
        end
        Rails.logger.silence(Logger::INFO) do
          Entity.import!(columns, entities) unless entities.empty?
          update_entities_list(count)
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

  def estimated_duration
    if EntitiesImport.success.any?
      d = EntitiesImport.success.limit(10).average(:duration)
      f = EntitiesImport.success.limit(10).average(:filesize)
      estimation = ((filesize * d) / f)
      {
        available: true,
        duration: estimation,
        start: created_at.to_i,
        end: created_at.to_i + estimation.to_i
      }
    else
      {
        available: false
      }
    end
  end


  private

    def absence_of_concurrent_import
      if entities_list.entities_imports.running.any?
        errors.add(:base, I18n.t('errors.entities_import.concurrent_import'))
      end
    end

    def csv_reader_options
      {
        headers: [
          I18n.t('activerecord.attributes.entity.terms'),
          I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
          I18n.t('activerecord.attributes.entity.solution')
        ],
        skip_blanks: true,
        encoding: 'UTF-8'
      }
    end

    def csv_file_path
      if file.storage.is_a? Shrine::Storage::FileSystem
        file.storage.path(file.id)
      else # for test environment when Shrine uses in-memory storage
        file_path = Rails.root.join('tmp', 'test_vikyai_entities_import.csv')
        File.delete(file_path) if File.exists? file_path
        tempfile = File.open(file_path, 'w+')
        file.open { |f| tempfile.write(f.read) }
        tempfile.close
        tempfile.path
      end
    end

    def csv_count_lines
      CSV.foreach(csv_file_path, csv_reader_options).count - 1
    end

    def validate_csv_header!(header_row)
      if header_row['Terms'].downcase != 'terms' || header_row['Auto solution'].downcase != 'auto solution' || header_row['Solution'].downcase != 'solution'
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_header')
      end
    end

    def validate_csv_row_length!(row, row_number)
      return if row['Terms'].present? && row['Auto solution'].present? && row['Solution'].present?
      if row['Terms'].nil? || row['Auto solution'].nil?
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_column', row_number: row_number)
      end
      if !csv_auto_solution_to_auto_solution(row) && row['Solution'].blank?
        raise CSV::MalformedCSVError, I18n.t('errors.entities_import.missing_column', row_number: row_number)
      end
    end

    def build_import_line_from_csv_row(row, position, entities_list_id)
      terms = csv_terms_to_terms(row)
      auto_solution = csv_auto_solution_to_auto_solution(row)
      [
        terms,
        auto_solution,
        csv_solution_to_solution(row, terms, auto_solution),
        position,
        entities_list_id
      ]
    end

    def csv_terms_to_terms(row)
      terms = row['Terms']
      if terms.present?
        terms = terms.tr('|', "\n")
        terms = EntityTermsParser.new(terms).proceed
      end
      terms
    end

    def csv_auto_solution_to_auto_solution(row)
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

    def csv_solution_to_solution(row, terms, auto_solution)
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

    def update_entities_list(count)
      entities_list.agent.update_locales
      EntitiesList.update_counters(entities_list.id, entities_count: count)
      entities_list.touch
    end

end
