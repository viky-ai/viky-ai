class AddAgentVisibilityColumn < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :visibility, :integer, default: 0
  end
end
