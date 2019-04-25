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
    add_index :interpretations, [:intent_id, :position], unique: true
  end

  def down
    remove_index :interpretations, [:intent_id, :position]
  end
end
