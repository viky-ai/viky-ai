class AddFkContraintToAgentArcs < ActiveRecord::Migration[5.1]
  def change
    add_foreign_key :agent_arcs, :agents, column: :source_id, on_delete: :cascade
    add_foreign_key :agent_arcs, :agents, column: :target_id, on_delete: :cascade
  end
end
