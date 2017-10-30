# encoding: utf-8

module Nls

  class Alias

    attr_reader :name
    attr_reader :interpretation
    attr_accessor :expression

    def initialize(interpretation, name = nil)
      name = interpretation.slug if name.nil?
      @name = name
      @interpretation = interpretation
    end

    def to_h
      {
        "alias" => @name,
        "slug" => @interpretation.slug,
        "id" => @interpretation.id.to_s,
        "package" => @interpretation.package.id.to_s
      }
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

  end

end
