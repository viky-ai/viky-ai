class AddApiTokenToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :api_token, :string
    add_index :agents, :api_token, unique: true
  end
end
