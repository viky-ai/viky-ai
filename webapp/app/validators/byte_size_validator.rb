class ByteSizeValidator < ActiveModel::EachValidator
  def validate_each(record, attribute, value)
    if options.key?(:minimum)
      if value.bytesize < options[:minimum]
        record.errors.add attribute, :too_small, count: options[:minimum]
      end
    elsif options.key?(:maximum)
      if value.bytesize > options[:maximum]
        record.errors.add attribute, :too_large, count: options[:maximum]
      end
    end
  end

end
