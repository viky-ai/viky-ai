class ConvertLocalesTypography < ActiveRecord::Migration[5.1]

  def up
    Nlp::Package.sync_active = false
    begin
      convert_interpretation('_', '-')
      convert_intent('_', '-')
    ensure
      Nlp::Package.sync_active = true
    end
  end

  private

    def convert_interpretation(current_separator, new_separator)
      ActiveRecord::Base.record_timestamps = false
      begin
        Interpretation.all.each do |interpretation|
          previous_locale = interpretation.locale
          interpretation.locale = previous_locale.gsub(/([a-z]{2})#{current_separator}([A-Z]{2})/, '\1'+new_separator+'\2')
          interpretation.save!
        end
      ensure
        ActiveRecord::Base.record_timestamps = true
      end
    end

    def convert_intent(current_separator, new_separator)
      ActiveRecord::Base.record_timestamps = false
      begin
        Intent.all.each do |intent|
          previous_locales = intent.locales
          intent.locales = previous_locales.map { |locale| locale.gsub(/([a-z]{2})#{current_separator}([A-Z]{2})/, '\1'+new_separator+'\2') }
          intent.save!
        end
      ensure
        ActiveRecord::Base.record_timestamps = true
      end
    end

end
