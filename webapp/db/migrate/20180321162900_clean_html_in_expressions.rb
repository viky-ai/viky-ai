class CleanHtmlInExpressions < ActiveRecord::Migration[5.1]
  def change
    ActiveRecord::Base.record_timestamps = false
    begin
      Interpretation.find_each do |interpretation|
        interpretation.expression = ActionController::Base.helpers.strip_tags(interpretation.expression)
        interpretation.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
