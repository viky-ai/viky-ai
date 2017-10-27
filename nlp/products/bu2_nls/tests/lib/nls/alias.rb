# encoding: utf-8

module Nls

  class Alias

    attr_reader :stringAlias
    attr_reader :slug
    attr_reader :id
    attr_reader :package
    attr_accessor :expression

    def initialize(stringAlias, slug, id, package)
      @stringAlias = stringAlias
      @slug = slug
      @id = id
      @package = package
    end

    def to_h
      {
        "alias" => @stringAlias,
        "slug" => @slug,
        "id" => @id,
        "package" => @package
      }
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

  end

end
