class AddUniqueConstraintToIntent < ActiveRecord::Migration[5.1]
  def change
    add_index(:intents, [:intentname, :agent_id], unique: true)
  end
end
