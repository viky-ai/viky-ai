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

    def first()
       @answers[0]
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
