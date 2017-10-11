class AgentSearch
  attr_reader :options

  def initialize(options = {})
    @options = options || {}
  end

  def self.keys
    ["query",]
  end

  keys.each do |meth|
    define_method(meth) { options[meth.to_sym] }
  end

  def empty?
    is_empty = true
    options.each do |key, value|
      unless [:user_id].include?(key)
        is_empty = false unless value.blank?
      end
    end
    return is_empty
  end
end
