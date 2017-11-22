class AddColumnToIntents < ActiveRecord::Migration[5.1]
  def change
    add_column :intents, :locales, :string

    ActiveRecord::Base.record_timestamps = false
    Nlp::Package.sync_active = false
    begin
      Intent.all.each do |intent|
        locales = intent.interpretations.group(:locale).count
        intent.locales = locales.keys.any? ? locales.keys.collect{|k| k.gsub('_', '-')} : ['fr-FR']
        intent.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
      Nlp::Package.sync_active = true
    end
  end
end
