class AddApiTokenToAgents < ActiveRecord::Migration[5.1]
  def up
    add_column :agents, :api_token, :string
    add_index :agents, :api_token, unique: true

    ActiveRecord::Base.record_timestamps = false
    begin
      Agent.all.each do |agent|
        if agent.api_token.nil?
          agent.ensure_api_token
          agent.save!
        end
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end

  def down
    remove_index :agents, :api_token
    remove_column :agents, :api_token
  end
end
