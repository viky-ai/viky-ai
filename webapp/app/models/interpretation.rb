class Interpretation < ApplicationRecord
  LOCALES = %w[* en fr es pt zh ar].freeze

  belongs_to :intent, touch: true
  has_many :interpretation_aliases, dependent: :destroy

  accepts_nested_attributes_for :interpretation_aliases, allow_destroy: true

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::LOCALES }, presence: true
  validates :solution, length: { maximum: 2000 }

  before_save :cleanup
  before_create :set_position

  def is_minimal
    interpretation_aliases.count == 0 && auto_solution_enabled
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
      self.expression = expression.strip unless expression.nil?
      if auto_solution_enabled
        self.solution = nil
      elsif solution.blank?
        self.solution = ''
      end
    end

    def set_position
      return if intent.nil?
      self.position = intent.interpretations_with_local(locale).count
    end
end
