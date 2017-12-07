class AddUnicityToAgentArc < ActiveRecord::Migration[5.1]
  def change
    add_index(:agent_arcs, [:source_id, :target_id], unique: true)
  end
end
