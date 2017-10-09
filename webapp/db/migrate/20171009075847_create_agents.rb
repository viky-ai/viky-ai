class CreateAgents < ActiveRecord::Migration[5.1]
  def change
    create_table :agents, id: :uuid do |t|
      t.string :name
      t.text :description
      t.string :agentname

      t.timestamps
    end
  end
end
