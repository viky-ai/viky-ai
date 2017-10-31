# encoding: utf-8

module Nls

  class Expression
    attr_reader :expression
    attr_reader :locale
    attr_accessor :interpretation

    def initialize(expression, locale = "fr-FR")
      @expression = expression
      @locale = locale
    end

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
