class AddPositionToEntities < ActiveRecord::Migration[5.1]
  def change
    add_column :entities, :position, :integer, default: 0
  end
end
