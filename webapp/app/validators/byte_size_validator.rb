class ByteSizeValidator < ActiveModel::EachValidator
  include ActionView::Helpers::NumberHelper

  def validate_each(record, attribute, value)
    return if value.nil?

    actual_size = value.bytesize
    if options.key?(:minimum)
      if actual_size < options[:minimum]
        record.errors.add attribute, :too_small, count: format_bytes(options[:minimum]), actual: format_bytes(actual_size)
      end
    end
    if options.key?(:maximum)
      if actual_size > options[:maximum]
        record.errors.add attribute, :too_large, count: format_bytes(options[:maximum]), actual: format_bytes(actual_size)
      end
    end
  end

  private
    def format_bytes(bytes)
      number_to_human_size(bytes, precision: 4)
    end
end
