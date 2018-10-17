# encoding: utf-8

module Nls

  class Expression
    attr_reader :expression
    attr_reader :locale
    attr_reader :aliases
    attr_reader :keep_order
    attr_reader :glued
    attr_accessor :solution
    attr_accessor :interpretation

    def initialize(expression, opts = {})
      @expression = expression

      @locale = nil
      @locale = opts[:locale] if opts.has_key?(:locale)

      @keep_order = false
      @keep_order = opts[:keep_order] if opts.has_key?(:keep_order)

      @glued = false
      @glued = opts[:glued] if opts.has_key?(:glued)

      @solution = nil
      @solution = opts[:solution] if opts.has_key?(:solution)

      @aliases = []
      if opts.has_key?(:aliases)
        aliases = opts[:aliases]

        if aliases.kind_of? Array

          aliases.each do |_alias|
            add_alias(_alias)
          end

        elsif aliases.kind_of? Hash

          # hash of alias_name => Alias | Interpretation
          aliases.each do |alias_name, _alias|

            if _alias.kind_of? Alias
              add_alias(_alias)
            elsif _alias.kind_of? AliasSpecificType
              new_alias = Alias.new(nil, { name: alias_name, type: _alias.type, regex: _alias.regex })
              add_alias(new_alias)
            elsif _alias.kind_of? Interpretation
              interpretation = _alias
              new_alias = Alias.new(interpretation, { name: alias_name })
              add_alias(new_alias)
            else
              raise "Unsupported alias : #{alias_name} => #{_alias}"
            end

          end

        else
          raise "Unsupported alias : #{aliases}"
        end
      end


    end

    def add_alias(new_alias)
      if !new_alias.kind_of? Alias
        intepretation_name = ""
        intepretation_name = "#{@interpretation.package.slug}/#{@interpretation.slug}/" if !@interpretation.nil?
        raise "Alias (#{new_alias}, #{new_alias.class}) added must a #{Alias.name} in expression (#{intepretation_name}#{@expression})"
      end

      @aliases << new_alias
      new_alias.expression = self
    end
    alias_method '<<', 'add_alias'


    def to_h
      hash = {}
      hash['expression'] = @expression
      hash['keep-order'] = true if @keep_order
      hash['glued'] = true if @glued
      hash['aliases'] = @aliases.map{|a| a.to_h} if !@aliases.empty?
      hash['locale'] = @locale if !@locale.nil?
      if !@solution.nil?
        if @solution.respond_to?(:to_h)
          hash['solution'] = @solution.to_h.deep_stringify_keys
        else
          hash['solution'] = @solution
        end
      end
      hash
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

    def self.no_locale
      nil
    end

    def self.keep_order
      true
    end

    def self.glued
      true
    end

    def self.no_order
      false
    end

  end
end
