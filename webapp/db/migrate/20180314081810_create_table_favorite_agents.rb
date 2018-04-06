class CreateTableFavoriteAgents < ActiveRecord::Migration[5.1]
  def change
    create_table :favorite_agents, id: :uuid do |t|
      t.references :user, foreign_key: { on_delete: :cascade }, type: :uuid
      t.references :agent, foreign_key: { on_delete: :cascade }, type: :uuid

      t.timestamps
    end
    add_index(:favorite_agents, [:user_id, :agent_id], unique: true)
  end
end
