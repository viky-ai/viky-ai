class InterpretationAlias < ApplicationRecord

  belongs_to :interpretation

  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }

  validate :check_position_start_greater_than_end


  private

    def check_position_start_greater_than_end
      if self.position_start >= self.position_end
        errors.add(:position_end, I18n.t('errors.interpretation_aliase.end_position_lower_than_start'))
      end
    end
end
