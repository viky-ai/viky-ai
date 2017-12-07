class AddOptionsToInterpretation < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretations, :keep_order, :boolean, default: false
    add_column :interpretations, :glued, :boolean, default: false
    add_column :interpretations, :solution, :text, limit: 2000
  end
end
