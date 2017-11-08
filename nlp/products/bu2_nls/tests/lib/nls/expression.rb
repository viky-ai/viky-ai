# encoding: utf-8

module Nls

  class Expression
    attr_reader :expression
    attr_reader :locale
    attr_reader :aliases
    attr_reader :keep_order
    attr_reader :solutions
    attr_accessor :interpretation

    def initialize(expression, aliases = [],locale = nil, keep_order = nil)
      @expression = expression
      @locale = locale
      @keep_order = nil
      @keep_order = 1 if keep_order == "true"

      @solutions = nil

      @aliases = []
      if aliases.kind_of? Array
        aliases.each do |_alias|
          add_alias(_alias)
        end
      elsif aliases.kind_of? Hash
        # hash of alias_name => interpretation
        aliases.each do |alias_name, interpretation|
          if interpretation.nil?
            _alias = Alias.new(interpretation, alias_name, "true")
          else
            _alias = Alias.new(interpretation, alias_name)
          end
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

    def add_solution(key,value)
      if @solutions.nil?
        @solutions = Solutions.new(key,value)
      else
        @solutions.add_solution(key,value)
      end
      self
    end


    def to_h
      hash = {}
      hash['expression'] = @expression
      hash['locale'] = @locale if !@locale.nil?
      hash['keep-order'] = true if !@keep_order.nil?
      hash['aliases'] = @aliases.map{|a| a.to_h} if !@aliases.empty?
      hash["solution"] = @solutions.to_h if !@solutions.nil?
      hash
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

    def self.no_locale
      nil
    end

    def self.keep_order
      "true"
    end

    def self.no_order
      nil
    end

  end
end
