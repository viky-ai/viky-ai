class AddColumnToIntents < ActiveRecord::Migration[5.1]
  def change
    add_column :intents, :locales, :string

    ActiveRecord::Base.record_timestamps = false
    begin
      Intent.all.each do |intent|
        locales = intent.interpretations.group(:locale).count
        intent.locales = locales.keys.any? ? locales.keys : ['fr_FR']
        intent.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
