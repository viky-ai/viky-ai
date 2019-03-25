class AddMissingLocalesInAgents < ActiveRecord::Migration[5.1]
  def up
    ActiveRecord::Base.record_timestamps = false
    begin
      Agent.all.each do |agent|
        if agent.locales.empty?
          agent.locales = [Locales::ANY]
          agent.save!
        end
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end

  def down
  end

end
