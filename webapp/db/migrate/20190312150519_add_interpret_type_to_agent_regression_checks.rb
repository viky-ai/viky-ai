class AddInterpretTypeToAgentRegressionChecks < ActiveRecord::Migration[5.1]
  def up
    ActiveRecord::Base.record_timestamps = false
    begin
      AgentRegressionCheck.find_each do |regression_check|
        expected_id = regression_check.expected['id']
        regression_check.expected['root_type'] = Intent.exists?(expected_id) ? 'intent' : 'entities_list'

        if regression_check.got.present?
          got_id = regression_check.got['id']
          regression_check.got['root_type'] = Intent.exists?(got_id) ? 'intent' : 'entities_list'
        end
        regression_check.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end

  def down
    ActiveRecord::Base.record_timestamps = false
    begin
      AgentRegressionCheck.find_each do |regression_check|
        regression_check.expected.delete('root_type')
        regression_check.got.delete('root_type') if regression_check.got.present?
        regression_check.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
