class CreateEntitiesImports < ActiveRecord::Migration[5.1]
  def change
    create_table :entities_imports, id: :uuid do |t|
      t.text :file_data
      t.integer :mode
      t.references :entities_list, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
