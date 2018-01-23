class CreateEntitiesLists < ActiveRecord::Migration[5.1]
  def change
    create_table :entities_lists, id: :uuid do |t|
      t.string :listname
      t.integer :position, default: 0
      t.string :color
      t.text :description
      t.integer :visibility, default: 0
      t.references :agent, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
