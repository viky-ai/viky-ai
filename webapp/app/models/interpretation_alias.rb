class InterpretationAlias < ApplicationRecord
  belongs_to :interpretation
  belongs_to :intent

  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }

  validate :check_position_start_greater_than_end

  # Validate intent_id is not interpretation.intent.id

  private

    def check_position_start_greater_than_end
      if self.position_start >= self.position_end
        errors.add(:position_end, I18n.t('errors.interpretation_aliase.end_position_lower_than_start'))
      end
    end
end
