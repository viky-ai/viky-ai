class RebuildEntitiesSolutions < ActiveRecord::Migration[5.1]
  def change
    ActiveRecord::Base.record_timestamps = false
    begin
      Entity.where(auto_solution_enabled: true).find_each do |entity|
        entity.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
