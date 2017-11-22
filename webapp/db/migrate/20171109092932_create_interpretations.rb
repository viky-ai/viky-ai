class CreateInterpretations < ActiveRecord::Migration[5.1]
  def change
    create_table :interpretations, id: :uuid do |t|
      t.string :expression
      t.references :intent, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
