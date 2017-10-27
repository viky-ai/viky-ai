# encoding: utf-8

module Nls

  class Expression
    attr_reader :expression
    attr_reader :locale
    attr_reader :aliases
    attr_accessor :interpretation

    def initialize(expression, locale = "fr-FR")
      @expression = expression
      @locale = locale

      @aliases = []
    end

    def add_alias(new_alias)
      if !new_alias.link_of Alias
        raise "Alias (#{new_alias}, #{new_alias.class}) added must a #{Alias.name} in expression (#{@interpretation.package.slug}/#{@interpretation.slug}/#{@expression})"
      end

      @aliases << new_alias
      new_alias.expression = self
    end
    alias_method '<<', 'add_alias'

    def to_h
      {
        "expression" => @expression,
        "locale" => @locale
      }
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

  end
end
