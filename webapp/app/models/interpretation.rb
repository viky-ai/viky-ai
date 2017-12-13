class Interpretation < ApplicationRecord
  Locales = %w[fr en es pt zh ar *].freeze

  belongs_to :intent
  has_many :interpretation_aliases, dependent: :destroy

  accepts_nested_attributes_for :interpretation_aliases, allow_destroy: true

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::Locales }, presence: true
  validates :solution, length: { maximum: 2000 }

  before_save :cleanup
  before_create :set_position

  after_save do
    Nlp::Package.new(intent.agent).push
  end

  after_destroy do
    Nlp::Package.new(intent.agent).push
  end

  def expression_with_aliases
    return expression if interpretation_aliases.count == 0
    ordered_aliases = interpretation_aliases.order(position_start: :asc)
    result = []
    expression.split('').each_with_index do |character, index|
      interpretation_alias = ordered_aliases.first
      if interpretation_alias.nil? || index < interpretation_alias.position_start
        result << character
      end
      if !interpretation_alias.nil? && index == interpretation_alias.position_end - 1
        result << "@{#{interpretation_alias.aliasname}}"
        ordered_aliases = ordered_aliases.drop 1
      end
    end
    result.join('')
  end


  private

    def cleanup
      unless expression.nil?
        self.expression = expression.strip
      end
      if auto_solution_enabled
        self.solution = nil
      else
        self.solution = "" if solution.blank?
      end
    end

    def set_position
      unless intent.nil?
        self.position = intent.interpretations_with_local(self.locale).count
      end
    end
end
