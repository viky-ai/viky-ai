class InterpretationAlias < ApplicationRecord
  belongs_to :interpretation
  belongs_to :intent

  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }

  validate :check_position_start_greater_than_end
  validate :intent_no_loop_reference

  private

    def intent_no_loop_reference
      unless self.intent_id.blank?
        if self.interpretation.intent.id == self.intent_id
          errors.add(:intent, I18n.t('errors.interpretation_alias.intent_loop_reference'))
        end
      end
    end

    def check_position_start_greater_than_end
      if self.position_start >= self.position_end
        errors.add(:position_end, I18n.t('errors.interpretation_alias.end_position_lower_than_start'))
      end
    end
end
