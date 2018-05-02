class CreateBots < ActiveRecord::Migration[5.1]
  def change
    create_table :bots, id: :uuid do |t|
      t.string :name
      t.string :endpoint
      t.references :agent, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
