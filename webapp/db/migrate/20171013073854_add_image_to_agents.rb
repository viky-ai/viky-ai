class AddImageToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :image_data, :text
  end
end
