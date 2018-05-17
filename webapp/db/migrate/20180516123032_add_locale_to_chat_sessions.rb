class AddLocaleToChatSessions < ActiveRecord::Migration[5.1]
  def change
    add_column :chat_sessions, :locale, :string, default: "en-US"
  end
end
