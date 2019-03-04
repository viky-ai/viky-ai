class ByteSizeValidator < ActiveModel::EachValidator
  def validate_each(record, attribute, value)
    return if value.nil?

    actual_size = value.bytesize
    if options.key?(:minimum)
      if actual_size < options[:minimum]
        record.errors.add attribute, :too_small, count: options[:minimum], actual: actual_size
      end
    end
    if options.key?(:maximum)
      if actual_size > options[:maximum]
        record.errors.add attribute, :too_large, count: options[:maximum], actual: actual_size
      end
    end
  end
end
