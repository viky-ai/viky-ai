class ChangeForeignKeyOfIntent < ActiveRecord::Migration[5.1]
  def change
    remove_foreign_key :intents, :agents
    add_foreign_key :intents, :agents, on_delete: :cascade
  end
end
