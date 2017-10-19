class AddOwnerIdToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :owner_id, :uuid
  end
end
