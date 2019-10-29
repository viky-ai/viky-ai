class DenormalizeAgentSlug < ActiveRecord::Migration[6.0]
  def change
    add_column :agents, :slug, :string

    ActiveRecord::Base.record_timestamps = false
    begin
      Agent.find_each do |agent|
        agent.slug = "#{agent.owner.username}/#{agent.agentname}"
        agent.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
