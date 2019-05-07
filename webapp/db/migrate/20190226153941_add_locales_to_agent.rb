class Intent < ApplicationRecord
  serialize :locales, JSON
end

class AddLocalesToAgent < ActiveRecord::Migration[5.1]
  def up
    ActiveRecord::Base.record_timestamps = false
    begin
      add_column :agents, :locales, :jsonb
      Agent.reset_column_information
      Agent.all.each do |agent|
        intents_locales = agent.intents.pluck(:locales).flatten.uniq
        agent.locales = Locales::ALL.select { |l| intents_locales.include?(l) }
        agent.locales = [Locales::ANY, 'en', 'fr'] if agent.locales.blank?
        agent.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end

  def down
    remove_column :agents, :locales
  end
end
