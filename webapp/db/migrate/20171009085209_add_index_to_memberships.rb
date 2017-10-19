class AddIndexToMemberships < ActiveRecord::Migration[5.1]
  def change
    add_index(:memberships, [:agent_id, :user_id], unique: true)
  end
end
