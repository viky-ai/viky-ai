class CleanUpChatSession < ActiveRecord::Migration[5.1]
  def up
    ChatSession.destroy_all
  end

  def down
  end
end
