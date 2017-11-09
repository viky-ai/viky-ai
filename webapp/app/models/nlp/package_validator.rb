class Nlp::PackageValidator < ActiveModel::Validator

  def validate(record)
    record.errors.add(:id, "is empty") if record.options[:id].nil?
    record.errors.add(:agent_id, "is empty") if record.options[:agent_id].nil?
    record.errors.add(:slug, "is empty") if record.options[:slug].nil?

    unless record.options[:interpretations].kind_of? Array
      record.errors.add(:interpretations, "must be an array of interpretation objects, or an empty array")
    end
  end

end
