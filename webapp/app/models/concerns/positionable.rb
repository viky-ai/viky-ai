module Positionable
  extend ActiveSupport::Concern

  included do
    before_create :set_position
  end


  module ClassMethods
    attr_reader :ancestor_classname

    def positionable_ancestor(*attributes)
      parent_name = attributes.first
      @ancestor_classname = parent_name.to_s.freeze
      @touch_ancestor = attributes.size > 1 ? attributes.second[:touch] : true
    end

    def update_positions(parent, public_list, private_list = nil)
      if self.attribute_method? :visibility
        update_with_visibility(parent, public_list, private_list)
      else
        update_without_visibility(parent, public_list)
      end
      parent.need_nlp_sync if @ancestor_classname == 'agent'
      parent.touch if @touch_ancestor
    end

    private

      def build_parent_column(parent)
        parent.class.table_name[0..-2] + '_id'
      end

      def get_unique_index_name(parent)
        parent_column = build_parent_column(parent)
        index_def = connection.indexes(self.table_name).select do |index|
          index.columns.eql?([parent_column, 'position'])
        end
        index_def.first.nil? ? '' : index_def.first.name
      end

      def update_with_visibility(parent, public_list, private_list)
        parent_column = build_parent_column(parent)
        current_public_objs  = self.where(parent_column => parent.id, id: public_list).order(position: :asc)
        current_private_objs = self.where(parent_column => parent.id, id: private_list).order(position: :asc)
        Agent.no_touching do
          update_order(public_list, current_public_objs, parent, self.visibilities[:is_public])
          update_order(private_list, current_private_objs, parent, self.visibilities[:is_private])
        end
      end

      def update_without_visibility(parent, list)
        parent_column = build_parent_column(parent)
        current_objs  = self.where(parent_column => parent.id, id: list).order(position: :asc)
        Agent.no_touching do
          update_order(list, current_objs, parent)
        end
      end

      def update_order(new_ids, current, parent, visibility = nil)
        count = current.count
        parent_column = build_parent_column(parent)
        old_position_values = current.collect(&:position)
        transaction do
          if connection.index_exists?(self.table_name, [parent_column, :position])
            # defer the unique constraint on position until the end of this transaction
            index_name = get_unique_index_name(parent)
            connection.execute "SET CONSTRAINTS #{index_name} DEFERRED" unless index_name.blank?
          end
          current.each do |item|
            new_position = new_ids.find_index(item.id)
            unless new_position.nil?
              new_position_value = old_position_values[count - new_position - 1]
              item.update_columns(position: new_position_value, updated_at: Time.zone.now)
              item.update_attribute(:visibility, visibility) unless visibility.nil?
            end
          end
        end
      end
  end
    
  private

    def set_position
      ancestor = self.send(self.class.ancestor_classname)
      return if ancestor.nil?
      children_method = ActiveModel::Naming.plural(self)
      if self.position.zero?
        if ancestor.send(children_method).count.zero?
          self.position = 0
        else
          self.position = ancestor.send(children_method).maximum(:position) + 1
        end
      end
    end
end