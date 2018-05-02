class CreateChatSessions < ActiveRecord::Migration[5.1]
  def change
    create_table :chat_sessions, id: :uuid do |t|
      t.references :user, foreign_key: { on_delete: :cascade }, type: :uuid
      t.references :bot, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
  end
end
