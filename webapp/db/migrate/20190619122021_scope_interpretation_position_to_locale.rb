class ScopeInterpretationPositionToLocale < ActiveRecord::Migration[5.1]
  def up
    execute <<-SQL
      ALTER TABLE interpretations
        DROP CONSTRAINT IF EXISTS index_interpretations_on_intent_id_and_position;
    SQL
    execute <<-SQL
      ALTER TABLE interpretations
        ADD CONSTRAINT index_interpretations_on_intent_id_locale_and_position UNIQUE (intent_id, locale, position)
        DEFERRABLE INITIALLY IMMEDIATE;
    SQL
  end

  def down
    execute <<-SQL
      ALTER TABLE interpretations
        DROP CONSTRAINT IF EXISTS index_interpretations_on_intent_id_locale_and_position;
    SQL
    execute <<-SQL
      ALTER TABLE interpretations
        ADD CONSTRAINT index_interpretations_on_intent_id_and_position UNIQUE (intent_id, position)
        DEFERRABLE INITIALLY IMMEDIATE;
    SQL
  end
end
