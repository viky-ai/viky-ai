class AddColumnPositionToInterpretations < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretations, :position, :integer, default: 0
  end
end
