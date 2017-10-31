# encoding: utf-8

module Nls

  class Expression
    attr_reader :expression
    attr_reader :locale
    attr_reader :aliases
    attr_accessor :interpretation

    def initialize(expression, aliases = [], locale = nil)
      @expression = expression
      @locale = locale

      @aliases = []
      if aliases.kind_of? Array
        aliases.each do |_alias|
          add_alias(_alias)
        end
      elsif aliases.kind_of? Hash
        # hash of alias_name => interpretation
        aliases.each do |alias_name, interpretation|
          _alias = Alias.new(interpretation, alias_name)
          add_alias(_alias)
        end
      else
        raise
      end

    end

    def add_alias(new_alias)
      if !new_alias.kind_of? Alias
        raise "Alias (#{new_alias}, #{new_alias.class}) added must a #{Alias.name} in expression (#{@interpretation.package.slug}/#{@interpretation.slug}/#{@expression})"
      end

      @aliases << new_alias
      new_alias.expression = self
    end
    alias_method '<<', 'add_alias'

    def to_h
      hash = {}
      hash['expression'] = @expression
      hash['locale'] = @locale if !@locale.nil?
      hash['aliases'] = @aliases.map{|a| a.to_h} if !@aliases.empty?
      hash
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

  end
end
