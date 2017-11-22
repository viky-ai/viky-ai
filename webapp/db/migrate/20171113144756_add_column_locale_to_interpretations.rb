class AddColumnLocaleToInterpretations < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretations, :locale, :string

    ActiveRecord::Base.record_timestamps = false
    begin
      Interpretation.all.each do |interpretation|
        interpretation.locale = 'fr_FR'
        interpretation.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end
end
