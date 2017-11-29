class InterpretationAlias < ApplicationRecord
  belongs_to :interpretation
  belongs_to :intent

  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }
  validates :aliasname, presence: true

  validate :check_position_start_greater_than_end
  validate :intent_no_loop_reference
  validate :no_overlap
  validate :check_aliasname_uniqueness

  private

    def no_overlap
      unless interpretation.nil?
        if interpretation.interpretation_aliases.size > 1
          range = (position_start..position_end)
          interpretation.interpretation_aliases.each do |ialias|
            unless object_id == ialias.object_id
              if (ialias.position_start..ialias.position_end).overlaps?(range)
                errors.add(:position, I18n.t('errors.interpretation_alias.overlap'))
              end
            end
          end
        end
      end
    end

    def intent_no_loop_reference
      unless intent_id.blank? || interpretation_id.blank?
        if interpretation.intent.id == intent_id
          errors.add(:intent, I18n.t('errors.interpretation_alias.intent_loop_reference'))
        end
      end
    end

    def check_position_start_greater_than_end
      unless position_start.blank? || position_end.blank?
        if position_start >= position_end
          errors.add(:position_end, I18n.t('errors.interpretation_alias.end_position_lower_than_start'))
        end
      end
    end

    def check_aliasname_uniqueness
      return if interpretation.nil?
      return if interpretation.interpretation_aliases.size <= 1
      dup_found = false
      list_without_destroy = interpretation.interpretation_aliases.select { |item| !item._destroy }
      for ialias in list_without_destroy
        for item in list_without_destroy
          himself = ialias.position_start == item.position_start && ialias.position_end == item.position_end
          unless himself
            dup_found ||= ialias.aliasname == item.aliasname
          end
          break if dup_found
        end
        break if dup_found
      end
      errors.add(:aliasname, 'has already been taken') if dup_found
    end
end
