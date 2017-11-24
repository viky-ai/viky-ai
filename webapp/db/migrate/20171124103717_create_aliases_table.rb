class CreateAliasesTable < ActiveRecord::Migration[5.1]
  def change
    create_table :interpretation_aliases, id: :uuid do |t|
      t.string :aliasname
      t.integer :position_start
      t.integer :position_end
      t.references :interpretation, foreign_key: { on_delete: :cascade }, type: :uuid
    end
  end
end
