class AddAutoSolutionEnabledToInterpretations < ActiveRecord::Migration[5.1]
  def up
    add_column :interpretations, :auto_solution_enabled, :boolean, default: true
    Interpretation.all.each do |interpretation|
      if interpretation.solution.blank?
        interpretation.solution = nil
      else
        interpretation.auto_solution_enabled = false
      end
      interpretation.save!
    end
  end

  def down
    remove_column :interpretations, :auto_solution_enabled
  end
end
