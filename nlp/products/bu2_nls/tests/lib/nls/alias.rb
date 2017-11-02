# encoding: utf-8

module Nls

  class Alias

    attr_reader :name
    attr_reader :interpretation
    attr_reader :is_any
    attr_accessor :expression

    def initialize(interpretation, name = nil, is_any = nil)
      if(interpretation.nil? && is_any.nil?)
        raise "Alias must have an interpretation except if is any"
      end
      if(name.nil? && is_any.nil?)
        raise "any alias must have a name"
      end
      if(is_any.nil?)
        name = interpretation.slug if name.nil?
        @name = name
        @interpretation = interpretation
        @is_any =  nil
      else
        @is_any = 1
        @name = name
      end
    end

    def to_h
      hash = {}
      if @is_any.nil?
        hash['alias'] = @name
        hash['slug'] = @interpretation.slug
        hash['id'] = @interpretation.id.to_s
        hash['package'] = @interpretation.package.id.to_s
      else
        hash['alias'] = @name
        hash['type'] = "any"
      end
      hash
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

    def self.any
      nil
    end

  end

end
