class Backend::UserSearch
  attr_reader :options

  def initialize(options = {})
    @options = clean_options options
    @options[:sort_by] = sort_by
  end

  def self.keys
    ["email", "status"]
  end

  keys.each do |meth|
    define_method(meth) { options[meth.to_sym] }
  end

  def sort_by
    options[:sort_by] || "last_action"
  end

  def empty?
    is_empty = true
    options.each do |key, value|
      unless [:sort_by].include?(key)
        is_empty = false if value.present?
      end
    end
    return is_empty
  end

  private

    def clean_options(options)
      (options || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end
    end

end
