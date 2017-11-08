# encoding: utf-8

module Nls

  class Interpretation

    @@default_locale = "fr-FR"
    attr_reader :id
    attr_reader :slug
    attr_reader :expressions
    attr_reader :solutions
    attr_accessor :package

    def initialize(slug, opts = {})

      @slug = slug

      # default values
      opts[:id] = UUIDTools::UUID.timestamp_create if !opts.has_key?(:id)

      @id = opts[:id]

      @expressions = []

      @solutions = nil

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
      hash = {}
      hash["id"] = @id.to_s
      hash["slug"] = @slug
      hash["expressions"] = @expressions.map{|v| v.to_h}
      hash["solution"] = @solutions.to_h if !@solutions.nil?
      hash
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

    def new_expression(text, aliases = [], locale = nil, keep_order = nil)
      add_expression(Expression.new(text, aliases, locale, keep_order))
      self
    end

    def new_textual(texts = [], locale = @@default_locale)
      texts.each do |t|
        add_expression(Expression.new(t, [], locale))
      end
      self
    end

    def add_solution(key,value)
      if @solutions.nil?
        @solutions = Solutions.new(key,value)
      else
        @solutions.add_solution(key,value)
      end
      self
    end

    def self.default_locale=(locale)
      @@default_locale = locale
    end

    def self.default_locale
      @@default_locale
    end

  end
end

