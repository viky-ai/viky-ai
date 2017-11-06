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
      @solutions = {}
      @solutions = solution if !solution.nil?
    end

    def add_solution(tag, value)
      @solutions[tag] = value
    end

    def clear_solutions
      @solutions = {}
    end

    def to_h
      hash_interpretation = {}
      hash_interpretation['package'] = @package
      hash_interpretation['id'] = @id
      hash_interpretation['slug'] = @slug
      hash_interpretation['score'] = @score
      hash_interpretation['solution'] = @solutions if !@solutions.empty?
      hash_interpretation
    end


  end
end
