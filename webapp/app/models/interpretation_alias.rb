class InterpretationAlias < ApplicationRecord
  enum nature: [:type_intent, :type_number, :type_entities_list]

  belongs_to :interpretation, touch: true
  belongs_to :interpretation_aliasable, polymorphic: true, optional: true, touch: true

  validates :position_start, numericality: { only_integer: true, greater_than_or_equal_to: 0 }
  validates :position_end, numericality: { only_integer: true, greater_than: 0 }
  validates :aliasname, presence: true

  validate :interpretation_aliasable_present, unless: -> { self.type_number? }
  validate :check_position_start_greater_than_end
  validate :no_overlap
  validate :check_aliasname_uniqueness
  validate :check_aliasname_valid_javascript_variable


  private

    def interpretation_aliasable_present
      return unless interpretation_aliasable.nil?
      if type_intent?
        errors.add(:intent, I18n.t('errors.interpretation_alias.nature_blank'))
      else
        errors.add(:entities_list, I18n.t('errors.interpretation_alias.nature_blank'))
      end
    end

    def no_overlap
      return if interpretation.nil?
      if interpretation.interpretation_aliases.size > 1
        range = (position_start..position_end)
         interpretation.interpretation_aliases
          .reject do |ialias|
            if !id.nil? || !ialias.id.nil?
              id == ialias.id
            else
              position_start == ialias.position_start &&
                position_end == ialias.position_end &&
                aliasname == ialias.aliasname
            end
          end
          .select { |ialias| (ialias.position_start..ialias.position_end).overlaps?(range) }
          .each { errors.add(:position, I18n.t('errors.interpretation_alias.overlap')) }
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
