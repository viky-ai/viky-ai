# encoding: utf-8

module Nls

  class Alias

    attr_reader :name
    attr_reader :interpretation
    attr_reader :type
    attr_accessor :expression

    def initialize(interpretation, opts = {}) # name = nil, is_any = nil)
      @type = "normal"
      @type = opts[:type] if opts.has_key?(:type)

      name = nil
      name = opts[:name] if opts.has_key?(:name)

      if(interpretation.nil? && @type == "normal")
        raise "Normal alias must have an interpretation"
      end
      if(name.nil? && @type == "any")
        raise "Alias with type any must have a name"
      end
      if(name.nil? && @type == "digit")
        raise "Alias with type digit must have a name"
      end
      if(@type == "normal")
        name = interpretation.slug if name.nil?
        @name = name
        @interpretation = interpretation
      else
        @name = name
      end
    end

    def to_h
      hash = {}
      if @type == "normal"
        hash['alias'] = "#{@name}"
        hash['slug'] = @interpretation.slug
        hash['id'] = @interpretation.id.to_s
        hash['package'] = @interpretation.package.id.to_s
      else
        hash['alias'] = "#{@name}"
        hash['type'] = "#{@type}"
      end
      hash
    end

    def to_json(options = {})
      to_h.to_json(options)
    end

    def self.any
      "any"
    end

    def self.digit
      "digit"
    end

  end

end
