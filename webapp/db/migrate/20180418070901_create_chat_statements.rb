class CreateChatStatements < ActiveRecord::Migration[5.1]
  def change
    create_table :chat_statements, id: :uuid do |t|
      t.integer :speaker
      t.integer :nature, default: 0
      t.string :content
      t.references :chat_session, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
