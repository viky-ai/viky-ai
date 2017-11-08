# encoding: utf-8

module Nls

  class Solutions
    attr_reader :solutions

    def initialize(key,value)
      @solutions = {}
      @solutions[key] = value
    end

    def to_h
      @solutions
    end

    def add_solution(key,value)
      @solutions[key] = value
    end

  end
end
