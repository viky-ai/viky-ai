class AddColumnColorToIntentTable < ActiveRecord::Migration[5.1]
  def change
    add_column :intents, :color, :string

    ActiveRecord::Base.record_timestamps = false
    begin
      Intent.all.each do |intent|
        intent.color = Intent::AVAILABLE_COLORS[Random.new.rand(0..Intent::AVAILABLE_COLORS.size-1)] if intent.color.blank?
        intent.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
