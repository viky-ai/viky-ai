class AddSourceAgentToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :source_agent, :string
  end
end
