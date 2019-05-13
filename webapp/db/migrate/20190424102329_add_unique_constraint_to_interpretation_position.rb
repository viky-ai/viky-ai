class AddUniqueConstraintToInterpretationPosition < ActiveRecord::Migration[5.1]
  def up
    Intent.find_each do |i|
      unless i.interpretations.select('position').group(:position).having("count(*) > 1").empty?
        count = i.interpretations.count
        position = count
        i.interpretations.order('position DESC').each do |interpretation|
          interpretation.update_columns(position: position)
          position = position - 1
        end
      end
    end
   
    execute <<-SQL
      ALTER TABLE interpretations
        ADD CONSTRAINT index_interpretations_on_intent_id_and_position UNIQUE (intent_id, position)
        DEFERRABLE INITIALLY IMMEDIATE;
    SQL
  end

  def down
    execute <<-SQL
      ALTER TABLE interpretations
        DROP CONSTRAINT IF EXISTS index_interpretations_on_intent_id_and_position;
    SQL
  end
end
