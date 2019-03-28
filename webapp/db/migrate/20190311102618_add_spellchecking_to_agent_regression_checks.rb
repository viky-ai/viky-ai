class AddSpellcheckingToAgentRegressionChecks < ActiveRecord::Migration[5.1]
  def change
    add_column :agent_regression_checks, :spellchecking, :integer

    ActiveRecord::Base.record_timestamps = false
    begin
      AgentRegressionCheck.find_each do |regression_check|
        regression_check.spellchecking = 1
        regression_check.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
