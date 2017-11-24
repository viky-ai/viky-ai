class Interpretation < ApplicationRecord
  Locales = %w[fr-FR en-US *].freeze

  belongs_to :intent

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::Locales }, presence: true
  validates :solution, length: { maximum: 2000 }

  validate :solution_json_valid

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

    def solution_json_valid
      begin
        JSON.parse(self.solution) unless self.solution.nil? || self.solution.empty?
      rescue JSON::ParserError
        errors.add(:solution, I18n.t('errors.interpretation.invalid_json'))
      end
    end
end
