class CreateIntents < ActiveRecord::Migration[5.1]
  def change
    create_table :intents, id: :uuid do |t|
      t.string :intentname
      t.text :description
      t.references :agent, foreign_key: true, type: :uuid

      t.timestamps
    end
  end
end
