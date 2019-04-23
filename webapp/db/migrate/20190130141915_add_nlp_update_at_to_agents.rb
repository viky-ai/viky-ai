class AddNlpUpdateAtToAgents < ActiveRecord::Migration[5.1]
  def change
    add_column :agents, :nlp_updated_at, :timestamp
  end
end
