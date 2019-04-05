class AddIndexToEntityPosition < ActiveRecord::Migration[5.1]
  def up
    EntitiesList.find_each do |el|
      unless el.entities.select('position').group(:position).having("count(*) > 1").empty?
        count = el.entities.count
        position = count
        el.entities.order('position DESC').each do |entity|
          entity.update_columns(position: position)
          position = position - 1
        end
      end
    end
    add_index :entities, [:entities_list_id, :position], unique: true
  end

  def down
    remove_index :entities, [:entities_list_id, :position]
  end
end
