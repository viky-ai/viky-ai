class Nls::InterpretValidator < ActiveModel::Validator

  def validate(record)
    record.errors.add(:sentence, "is empty") if record.options['sentence'].nil?
  end

end
