# encoding: utf-8

module Nls

  class Answer

    attr_reader :package
    attr_reader :id
    attr_reader :slug
    attr_reader :score
    attr_reader :solutions

    def initialize(interpretation, solution = nil)
      @package = interpretation.package.id.to_s
      @id = interpretation.id.to_s
      @slug = interpretation.slug
      @score = 1.0
      @solutions = solution
    end

    def add_solution(tag, value)
      if @solutions.nil?
        @solutions = Solutions.new(tag, value)
      else
        @solutions.add_solution(key,value)
      end
      self
    end

    def clear_solutions
      @solutions = nil
    end

    def to_h
      hash_interpretation = {}
      hash_interpretation['package'] = @package
      hash_interpretation['id'] = @id
      hash_interpretation['slug'] = @slug
      hash_interpretation['score'] = @score
      hash_interpretation['solution'] = @solutions.to_h if !@solutions.nil?
      hash_interpretation
    end


  end
end
