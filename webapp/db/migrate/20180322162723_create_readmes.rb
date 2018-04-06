class CreateReadmes < ActiveRecord::Migration[5.1]
  def change
    create_table :readmes, id: :uuid do |t|
      t.text :content
      t.references :agent, foreign_key: { on_delete: :cascade }, type: :uuid
      t.timestamps
    end
  end
end
