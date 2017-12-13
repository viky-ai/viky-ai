class InterpretationAlias < ApplicationRecord
  enum nature: [ :type_intent, :type_digit ]

  belongs_to :interpretation
  belongs_to :intent, optional: true

  validates :intent, presence: true, if: -> { self.type_intent? }
  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }
  validates :aliasname, presence: true

  validate :check_position_start_greater_than_end
  validate :no_overlap
  validate :check_aliasname_uniqueness
  validate :check_aliasname_valid_javascript_variable


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
        himself = ialias.position_start == position_start && ialias.position_end == position_end
        unless himself
          dup_found ||= ialias.aliasname == aliasname
        end
        break if dup_found
      end
      errors.add(:aliasname, I18n.t('errors.interpretation_alias.aliasname_uniqueness')) if dup_found
    end

    def check_aliasname_valid_javascript_variable
      errors.add(:aliasname, I18n.t('errors.interpretation_alias.aliasname_valid_javascript_variable')) unless aliasname =~ /\A[a-zA-Z$_][a-zA-Z0-9$_]*\z/
      javascript_reserved_keywords = %w(await break case catch class const continue debugger default delete do else enum export extends finally for function if implements import in instanceof interface let new package private protected public return static super switch this throw try typeof var void while with yield)
      errors.add(:aliasname, I18n.t('errors.interpretation_alias.aliasname_valid_javascript_variable')) if javascript_reserved_keywords.any? { |item| item == aliasname }
    end
end
