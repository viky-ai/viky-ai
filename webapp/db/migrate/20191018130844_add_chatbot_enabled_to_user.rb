class AddChatbotEnabledToUser < ActiveRecord::Migration[6.0]
  def change
    add_column :users, :chatbot_enabled, :boolean, default: false
  end
end
