class AddForeignKeysToAgents < ActiveRecord::Migration[5.1]
  def change
    add_foreign_key :agents, :users, column: :owner_id
  end
end
