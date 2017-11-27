class Interpretation < ApplicationRecord
  Locales = %w[fr-FR en-US *].freeze

  belongs_to :intent
  has_many :interpretation_aliases, dependent: :destroy

  accepts_nested_attributes_for :interpretation_aliases

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::Locales }, presence: true
  validates :solution, length: { maximum: 2000 }

  validate :solution_json_valid
  validate :interpretation_aliases_no_overlap

  before_save :cleanup
  before_create :set_position


  after_save do
    Nlp::Package.new(intent.agent).push
  end

  after_destroy do
    Nlp::Package.new(intent.agent).push
  end


  private

    def cleanup
      unless expression.nil?
        self.expression = expression.strip
      end
    end

    def set_position
      unless intent.nil?
        self.position = intent.interpretations_with_local(self.locale).count
      end
    end

    def interpretation_aliases_no_overlap
      if self.interpretation_aliases.count > 1
        self.interpretation_aliases.each_with_index do |ialias|
          range = (ialias.position_start..ialias.position_end)
          self.interpretation_aliases.where.not(id: ialias.id).each do |iialias|
            if (iialias.position_start..iialias.position_end).overlaps?(range)
              errors.add(:interpretation_aliases, I18n.t('errors.interpretation.interpretation_aliases_overlap'))
            end
          end
        end
      end
    end

    def solution_json_valid
      begin
        JSON.parse(self.solution) unless self.solution.nil? || self.solution.empty?
      rescue JSON::ParserError
        errors.add(:solution, I18n.t('errors.interpretation.invalid_json'))
      end
    end
end
