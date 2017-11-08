class AddPositionToIntents < ActiveRecord::Migration[5.1]
  def change
    add_column :intents, :position, :integer, default: 0
  end
end
