class AddUniqueConstraintToAgent < ActiveRecord::Migration[5.1]
  def change
    add_index(:agents, [:owner_id, :agentname], unique: true)
  end
end
