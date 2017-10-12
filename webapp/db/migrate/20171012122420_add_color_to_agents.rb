class AddColorToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :color, :string, default: 'black'
  end
end
