class AddDeferrableConstraintToEntityPosition < ActiveRecord::Migration[5.1]
  def up
    remove_index :entities, [:entities_list_id, :position]

    execute <<-SQL
      ALTER TABLE entities
        ADD CONSTRAINT index_entities_on_entities_list_id_and_position UNIQUE (entities_list_id, position)
        DEFERRABLE INITIALLY IMMEDIATE;
    SQL
  end

  def down
    execute <<-SQL
      ALTER TABLE entities
        DROP CONSTRAINT IF EXISTS index_entities_on_entities_list_id_and_position;
    SQL

    add_index :entities, [:entities_list_id, :position], unique: true
  end
end
