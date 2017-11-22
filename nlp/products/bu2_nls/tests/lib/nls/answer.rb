# encoding: utf-8

module Nls

  class Answer

    attr_reader :package
    attr_reader :id
    attr_reader :slug
    attr_reader :score
    attr_accessor :solution

    def initialize(interpretation, solution = nil)
      @package = interpretation.package.id.to_s
      @id = interpretation.id.to_s
      @slug = interpretation.slug
      @score = 1.0
      @solution = solution
    end

    def to_h
      hash_interpretation = {}
      hash_interpretation['package'] = @package
      hash_interpretation['id'] = @id
      hash_interpretation['slug'] = @slug
      hash_interpretation['score'] = @score
      if !@solution.nil?
        if @solution.respond_to?(:to_h)
          hash_interpretation['solution'] = @solution.to_h
        else
          hash_interpretation['solution'] = @solution
        end
      end
      hash_interpretation
    end


  end
end
