# encoding: utf-8

module Nls

  class Answers

    def initialize(interpretation, solution = nil)
      @answers = []
      @answers << Answer.new(interpretation, solution)
    end

    def add_answer(interpretation, solution = nil)
      @answers << Answer.new(interpretation, solution)
    end

    def clear_all_solutions
      @answers.each do |answer|
        answer.clear_solutions
      end
    end

    def add_solution(solution_id, tag, value)
      @answers[solution_id].add_solution(tag, value)
    end

    def answer(id_answer)
      @answers[id_answer]
    end
    alias_method '[]', 'answer'

    def to_h
      hash = {}
      interpretations_array = []
      @answers.each do |answer|
        interpretations_array << answer.to_h
      end
      hash['interpretations'] = interpretations_array

      hash
    end

  end
end
