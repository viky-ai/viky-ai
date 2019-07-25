class EntitiesImport < ApplicationRecord
  belongs_to :entities_list
  belongs_to :user

  include EntitiesImportFileUploader::Attachment.new(:file)
  validates_presence_of :file, message: I18n.t('errors.entities_import.no_file'), on: :create
  validate :absence_of_concurrent_import, on: :create
  validates :mode, presence: true
  validate :check_owner_quota, on: :create

  enum mode: [:append, :replace]
  enum status: [ :running, :success, :failure ]

  def proceed
    count = 0
    entities_list_id = entities_list.id
    columns = [:terms, :auto_solution_enabled, :solution, :position, :entities_list_id, :searchable_terms]
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
        index = 0
        CSV.foreach(csv_file_path, csv_reader_options) do |row|
          if index == 0
            validate_csv_header!(row)
            index += 1
          else
            validate_csv_row!(row, count + 1)

            terms = csv_terms_to_terms(row)
            auto_solution = (row['Auto solution'].downcase == 'true')
            solution = csv_solution_to_solution(row, terms, auto_solution)
            position = max_position + lines_count - count
            searchable_terms = Entity.extract_searchable_terms(terms)
            entities << [terms, auto_solution, solution, position, entities_list_id, searchable_terms]
            if (index % 1000).zero?
              batch_import(columns, entities, errors, max_position, lines_count)
              entities = []
              Rails.logger.info "Processing entities import #{index}/#{lines_count}"
            end
            count += 1
            index += 1
          end
        end
        batch_import(columns, entities, errors, max_position, lines_count)
        update_entities_list(count)
      rescue ActiveRecord::ActiveRecordError => e
        errors[:file] << "#{e.message}"
        count = 0
        raise ActiveRecord::Rollback
      rescue CSV::MalformedCSVError => e
        errors[:file] << "Bad CSV format: #{e.message}"
        count = 0
        raise ActiveRecord::Rollback
      end
      if errors.any?
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

    def batch_import(columns, entities, errors, max_position, lines_count)
      unless entities.empty?
        Rails.logger.silence(Logger::INFO) do
          import = Entity.import(columns, entities)
          if import.failed_instances.any? && errors.size < 100
            import.failed_instances.each do |failed_instance|
              line = max_position + lines_count - failed_instance.position + 1
              msg = "Bad entity format: "
              msg << failed_instance.errors.full_messages.join(', ')
              msg << " in line #{line}."
              errors[:file] << msg
            end
          end
        end
      end
    end

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
        tempfile = Tempfile.open(["test", ".csv"])
        file.open { |f| tempfile.write(f.read) }
        tempfile.close
        tempfile.path
      end
    end

    def csv_count_lines
      CSV.foreach(csv_file_path, csv_reader_options).count - 1
    end

    def validate_csv_header!(header_row)
      if header_row['Terms'].downcase != 'terms' ||
         header_row['Auto solution'].downcase != 'auto solution' ||
         header_row['Solution'].downcase != 'solution'
        msg = I18n.t('errors.entities_import.missing_header')
        raise CSV::MalformedCSVError.new(msg, 0)
      end
    end

    def validate_csv_row!(row, row_number)
      if row['Terms'].nil?
        msg = I18n.t('errors.entities_import.missing_column')
        raise CSV::MalformedCSVError.new(msg, row_number)
      end
      if row['Auto solution'].nil?
        msg = I18n.t('errors.entities_import.missing_column')
        raise CSV::MalformedCSVError.new(msg, row_number)
      else
        if ['true', 'false'].include? row['Auto solution'].downcase
          if row['Auto solution'].downcase == 'false' && row['Solution'].blank?
            msg = I18n.t('errors.entities_import.missing_column')
            raise CSV::MalformedCSVError.new(msg, row_number)
          end
        else
          msg = I18n.t('errors.entities_import.unexpected_autosolution')
          raise CSV::MalformedCSVError.new(msg, row_number)
        end
      end
    end

    def csv_terms_to_terms(row)
      terms = row['Terms']
      if terms.present?
        EntityTermsParser.new(terms.tr('|', "\n")).proceed
      else
        terms
      end
    end

    def csv_solution_to_solution(row, terms, auto_solution)
      solution = row['Solution']
      if auto_solution && terms.present?
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

    def check_owner_quota
      entities_quota = ENV.fetch('VIKYAPP_ENTITIES_QUOTA') { nil }
      unless entities_quota == nil
        entities_quota = Integer(entities_quota)
        total = EntitiesList.joins(:agent).where("agents.owner_id = ?", entities_list.agent.owner_id).sum(:entities_count)
        if mode == 'replace'
          if csv_count_lines > entities_quota
            errors.add(:base, I18n.t('errors.entities_import.quota_replace', maximum: entities_quota, to_import: csv_count_lines))
          end
        else 
          if total + csv_count_lines > entities_quota
            errors.add(:base, I18n.t('errors.entities_import.quota', maximum: entities_quota, actual: total, to_import: csv_count_lines))
          end
        end
      end
    end
end
