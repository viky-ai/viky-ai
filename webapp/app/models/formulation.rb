require 'unicode/emoji'

class Formulation < ApplicationRecord
  include Positionable
  positionable_ancestor :intent

  belongs_to :intent, touch: true
  has_many :formulation_aliases, dependent: :destroy

  accepts_nested_attributes_for :formulation_aliases, allow_destroy: true

  enum proximity: ExpressionProximity::FORMULATION_PROXIMITIES, _prefix: :proximity
  validates :expression, presence: true, byte_size: { maximum: 2048 }
  validates :solution, byte_size: { maximum: 8192 }
  validates :locale, inclusion: { in: Locales::ALL }, presence: true
  validate :validate_aliases_any_and_list_options
  validate :validate_expression_nlp_length
  validate :validate_owner_quota, on: :create, if: -> { Feature.quota_enabled? }
  before_save :cleanup

  def is_minimal
    formulation_aliases.count == 0 && auto_solution_enabled
  end

  def expression_with_aliases
    return expression if formulation_aliases.size == 0
    ordered_aliases = formulation_aliases.reject(&:_destroy).sort_by(&:position_start)
    result = []
    expression.split(//).each_with_index do |character, index|
      formulation_alias = ordered_aliases.first
      if formulation_alias.nil? || index < formulation_alias.position_start
        result << character
      end
      if !formulation_alias.nil? && index == formulation_alias.position_end - 1
        result << "@{#{formulation_alias.aliasname}}"
        ordered_aliases = ordered_aliases.drop 1
      end
    end
    result.join
  end

  def proximity
    @proximity ||= ExpressionProximity.new(read_attribute(:proximity))
  end

  def update_locale(new_locale)
    previous_locale = locale
    self.locale = new_locale
    max_position = intent.formulations.where(locale: new_locale).maximum(:position)
    self.position = max_position.nil? ? 0 : max_position + 1
    save
    previous_locale
  end

  private

    def validate_aliases_any_and_list_options
      aliases = formulation_aliases.reject(&:marked_for_destruction?)
      if aliases.size == 1
        if aliases.first.any_enabled
          errors.add(:base, I18n.t('errors.formulation.one_alias_one_any'))
        end
      elsif aliases.size > 1
        if aliases.select {|a| a.any_enabled}.size > 1
          errors.add(:base, I18n.t('errors.formulation.only_one_any'))
        end
        if aliases.select {|a| a.is_list}.size > 1
          errors.add(:base, I18n.t('errors.formulation.only_one_list'))
        end
      end
    end

    def validate_expression_nlp_length
      nlp_max_length = 36
      exp = expression_with_aliases
              .gsub(/@{[a-zA-Z$_][a-zA-Z0-9$_]*}/, 'alias')
              .gsub(/(\D)(\d)/, '\1 \2')
              .gsub(/(\d)(\D)/, '\1 \2')
              .gsub(/(\p{No})/, ' \1 ') # G_UNICODE_OTHER_NUMBER (No)
              .gsub(/(\p{Pc})/, ' \1 ') # G_UNICODE_CONNECT_PUNCTUATION (Pc)
              .gsub(/(\p{Pd})/, ' \1 ') # G_UNICODE_DASH_PUNCTUATION (Pd)
              .gsub(/(\p{Ps})/, ' \1 ') # G_UNICODE_OPEN_PUNCTUATION (Ps)
              .gsub(/(\p{Pe})/, ' \1 ') # G_UNICODE_CLOSE_PUNCTUATION (Pe)
              .gsub(/(\p{Pi})/, ' \1 ') # G_UNICODE_INITIAL_PUNCTUATION (Pi)
              .gsub(/(\p{Pf})/, ' \1 ') # G_UNICODE_FINAL_PUNCTUATION (Pf)
              .gsub(/(\p{Po})/, ' \1 ') # G_UNICODE_OTHER_PUNCTUATION (Po)
              .gsub(/(\p{Sc})/, ' \1 ') # G_UNICODE_CURRENCY_SYMBOL (Sc)
              .gsub(/(\p{Sk})/, ' \1 ') # G_UNICODE_MODIFIER_SYMBOL (Sk)
              .gsub(/(\p{Sm})/, ' \1 ') # G_UNICODE_MATH_SYMBOL (Sm)
              .gsub(/(\p{So})/, ' \1 ') # G_UNICODE_OTHER_SYMBOL (So)
              .gsub(/([\u4e00-\u9FFF])/, ' \1 ') # G_UNICODE_BREAK_IDEOGRAPHIC (ID) https://en.wikipedia.org/wiki/CJK_Unified_Ideographs_(Unicode_block)
              .gsub(Unicode::Emoji::REGEX, ' emoji ') # G_UNICODE_BREAK_EMOJI_BASE (EB) https://en.wikipedia.org/wiki/Emoji#ref_U1F602_tr51
              .gsub(/\p{Cc}/, '') # G_UNICODE_CONTROL (Cc)
              .gsub(/\p{Cf}/, '') # G_UNICODE_FORMAT (Cf)
              .gsub(/\p{Cn}/, '') # G_UNICODE_UNASSIGNED (Cn)
              .gsub(/\p{Co}/, '') # G_UNICODE_PRIVATE_USE (Co)
              .gsub(/\p{Cs}/, '') # G_UNICODE_SURROGATE (Cs)
      actual_count = exp.split.size
      errors.add(:expression, I18n.t('errors.formulation.expression_nlp_length', count: nlp_max_length, actual_count: actual_count)) if actual_count > nlp_max_length
    end

    def validate_owner_quota
      unless intent.nil?
        owner = User.find(intent.agent.owner_id)
        if owner.quota_exceeded?
          errors.add(:quota, I18n.t('errors.formulation.quota', maximum: Quota.expressions_limit))
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
