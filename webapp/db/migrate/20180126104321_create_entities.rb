class CreateEntities < ActiveRecord::Migration[5.1]
  def change
    create_table :entities, id: :uuid do |t|
      t.text :solution, limit: 2000
      t.boolean :auto_solution_enabled, default: true
      t.text :terms, limit: 5000
      t.references :entities_list, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
