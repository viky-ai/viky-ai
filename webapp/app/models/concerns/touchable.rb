# Touches the associations of a model if specified fields change
module Touchable
  extend ActiveSupport::Concern

  included do
    before_save :check_touchable_associations
    after_save :touch_marked_associations
  end

  module ClassMethods
    def touch(association, options)
      @touchable_associations ||= {}
      @touchable_associations[association] = options
    end
  end

  private

  def touchable_associations
    self.class.instance_variable_get('@touchable_associations')
  end

  def update_association
    @update_association ||= {}
  end

  def check_touchable_associations
    return unless touchable_associations.present?

    touchable_associations.each_pair do |association, change_triggering_fields|
      if any_of_the_declared_field_changed?(change_triggering_fields)
        update_association[association] = true
      end
    end
  end

  def any_of_the_declared_field_changed?(options)
    (options[:if_attributes_changed] & changes.keys.map(&:to_sym)).present?
  end

  def touch_marked_associations
    return unless touchable_associations.present?

    touchable_associations.each_key do |association_key|
      next unless update_association[association_key]

      association = send(association_key)
      association.update_all(updated_at: Time.zone.now)
      update_association[association_key] = false
    end
  end
end
