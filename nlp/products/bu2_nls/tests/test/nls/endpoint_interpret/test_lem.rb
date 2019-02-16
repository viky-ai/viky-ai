# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLem < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr"


        lem_package = Package.new("lem")
        travailler = lem_package.new_interpretation("travailler")
        travailler << Expression.new("travailler", locale: "fr")

        cheval = lem_package.new_interpretation("cheval")
        cheval << Expression.new("cheval")

        Nls.package_update(lem_package)

      end


      def test_lem_simple

        sentence = "travaillerions"
        check_interpret(sentence, interpretation: "travailler", score: 0.99)

        sentence = "chevaux"
        check_interpret(sentence, interpretation: "cheval", score: 0.99)

      end

    end
  end
end
