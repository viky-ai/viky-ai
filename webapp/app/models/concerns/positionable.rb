module Positionable
  extend ActiveSupport::Concern

  included do
    before_create :set_position
  end


  module ClassMethods
    def positionable_class(p_class)
      @positionable_class = p_class
    end

    def update_positions(parent, public_list, private_list = nil)
      if @positionable_class.attribute_method? :visibility
        update_with_visibility(parent, public_list, private_list)
      else
        update_without_visibility(parent, public_list)
      end
    end


    private

      def build_parent_column(parent)
        parent.class.table_name[0..-2] + '_id'
      end

      def update_with_visibility(parent, public_list, private_list)
        parent_column = build_parent_column(parent)
        current_public_objs  = @positionable_class.where(parent_column => parent.id, id: public_list).order(position: :asc)
        current_private_objs = @positionable_class.where(parent_column => parent.id, id: private_list).order(position: :asc)
        Agent.no_touching do
          update_order(public_list, current_public_objs, @positionable_class.visibilities[:is_public])
          update_order(private_list, current_private_objs, @positionable_class.visibilities[:is_private])
        end
        parent.touch
      end

      def update_without_visibility(parent, list)
        parent_column = build_parent_column(parent)
        current_objs  = @positionable_class.where(parent_column => parent.id, id: list).order(position: :asc)
        Agent.no_touching do
          update_order(list, current_objs)
        end
        parent.touch
      end

      def update_order(new_ids, current, visibility = nil)
        count = current.count
        current.each do |item|
          new_position = new_ids.find_index(item.id)
          unless new_position.nil?
            item.record_timestamps = false
            item.update_attribute(:position, count - new_position - 1)
            item.record_timestamps = true
            item.update_attribute(:visibility, visibility) unless visibility.nil?
          end
        end
      end
  end


  private

    def set_position
      return if positionable_parent.nil?
      if self.position.zero?
        self.position = positionable_collection.count.zero? ? 0 : positionable_collection.maximum(:position) + 1
      end
    end
end
