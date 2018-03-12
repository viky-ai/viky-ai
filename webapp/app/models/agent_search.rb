class AgentSearch
  attr_reader :user_id, :options

  def initialize(user_id, options = {})
    @options = clean_options options
    @options[:user_id] = user_id.strip
    @options[:sort_by] = sort_by
  end

  def self.keys
    ['query']
  end

  keys.each do |meth|
    define_method(meth) { options[meth.to_sym] }
  end

  def sort_by
    options[:sort_by] || "name"
  end

  def empty?
    is_empty = true
    options.each do |key, value|
      unless [:user_id, :sort_by].include?(key)
        is_empty = false if value.present?
      end
    end
    is_empty
  end

  private

    def clean_options(options)
      (options || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end
    end
end
