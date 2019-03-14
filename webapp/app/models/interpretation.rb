class Interpretation < ApplicationRecord
  include Positionable
  positionable_ancestor :intent

  LOCALES = %w[* en fr es pt zh ar].freeze

  belongs_to :intent, touch: true
  has_many :interpretation_aliases, dependent: :destroy

  accepts_nested_attributes_for :interpretation_aliases, allow_destroy: true

  validates :expression, presence: true, byte_size: { maximum: 2048 }
  validates :solution, byte_size: { maximum: 8192 }
  validates :locale, inclusion: { in: self::LOCALES }, presence: true
  validate :check_aliases_any_and_list_options

  before_save :cleanup

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

    def check_aliases_any_and_list_options
      aliases = interpretation_aliases.reject(&:marked_for_destruction?)
      if aliases.size == 1
        if aliases.first.any_enabled
          errors.add(:base, I18n.t('errors.interpretation.one_alias_one_any'))
        end
      elsif aliases.size > 1
        if aliases.select {|a| a.any_enabled}.size > 1
          errors.add(:base, I18n.t('errors.interpretation.only_one_any'))
        end
        if aliases.select {|a| a.is_list}.size > 1
          errors.add(:base, I18n.t('errors.interpretation.only_one_list'))
        end
      end
    end

    def cleanup
      self.expression = ActionController::Base.helpers.strip_tags(expression.strip) unless expression.nil?
      if auto_solution_enabled
        self.solution = nil
      elsif solution.blank?
        self.solution = ''
      end
    end
end
