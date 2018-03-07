module Positionable
  extend ActiveSupport::Concern

  included do
    before_create :set_position
  end


  module ClassMethods
    def positionable_class(p_class)
      @positionable_class = p_class
    end

    def update_positions(agent, public_list, private_list)
      current_public_objs  = @positionable_class.where(agent_id: agent.id, id: public_list).order(position: :asc)
      current_private_objs = @positionable_class.where(agent_id: agent.id, id: private_list).order(position: :asc)
      Agent.no_touching do
        update_order(public_list, current_public_objs, @positionable_class.visibilities[:is_public])
        update_order(private_list, current_private_objs, @positionable_class.visibilities[:is_private])
      end
      agent.touch
    end


    private

      def update_order(new_ids, current, visibility)
        count = current.count
        current.each do |item|
          new_position = new_ids.find_index(item.id)
          unless new_position.nil?
            item.record_timestamps = false
            item.update_attribute(:position, count - new_position - 1)
            item.record_timestamps = true
            item.update_attribute(:visibility, visibility)
          end
        end
      end
  end


  private

    def set_position
      return if agent.nil?
      if self.position.zero?
        self.position = positionable_collection.count.zero? ? 0 : positionable_collection.maximum(:position) + 1
      end
    end
end
