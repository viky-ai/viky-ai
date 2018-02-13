class ChangeInterpretationLocales < ActiveRecord::Migration[5.1]
  def change
    ActiveRecord::Base.record_timestamps = false
    Nlp::Package.sync_active = false
    begin
      convert_interpretations
      convert_intents
    ensure
      ActiveRecord::Base.record_timestamps = true
      Nlp::Package.sync_active = true
    end

    Nlp::Package.reinit
  end

  private

    def convert_interpretations
      Interpretation.all.each do |interpretation|
        if interpretation.locale == 'fr-FR'
          interpretation.locale = 'fr'
        elsif interpretation.locale == 'en-US'
          interpretation.locale = 'en'
        end
        interpretation.save!
      end
    end

    def convert_intents
      Intent.all.each do |intent|
        intent.locales = intent.locales
                               .map { |locale| locale == 'fr-FR' ? 'fr' : locale }
                               .map { |locale| locale == 'en-US' ? 'en' : locale }
        intent.save!
      end
    end
end
