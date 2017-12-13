class CreateAgentArcs < ActiveRecord::Migration[5.1]
  def change
    create_table :agent_arcs do |t|
      t.uuid :source_id
      t.uuid :target_id
    end
  end
end
