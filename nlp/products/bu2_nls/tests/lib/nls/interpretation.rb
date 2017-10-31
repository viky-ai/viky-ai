# encoding: utf-8

module Nls

  class Interpretation

    attr_reader :id
    attr_reader :slug
    attr_reader :expressions
    attr_accessor :package

    def initialize(slug, opts = {})

      @slug = slug

      # default values
      opts[:id] = UUIDTools::UUID.timestamp_create if !opts.has_key?(:id)

      @id = opts[:id]

      @expressions = []

    end

    def add_expression(new_expression)

      if !new_expression.kind_of? Expression
        raise "Expression (#{new_expression}, #{new_expression.class}) added must a #{Expression.name} in interpretation (#{@package.slug}/#{@slug})"
      end

      @expressions << new_expression
      new_expression.interpretation = self
    end
    alias_method '<<', 'add_expression'

    def to_h
      {
        "id" => @id.to_s,
        "slug" => @slug,
        "expressions" => @expressions.map{|v| v.to_h}
      }
    end

    def to_match(score = 1.0)
      {
        "package" => package.id.to_s,
        "id" => @id.to_s ,
        "slug" => @slug,
        "score" => score
      }
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

  end
end

