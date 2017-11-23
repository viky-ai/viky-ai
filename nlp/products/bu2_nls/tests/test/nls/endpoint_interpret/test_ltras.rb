# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLtras < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil


        package = Package.new("ltras")
        ltras = package.new_interpretation("ltras")
        ltras << Expression.new("swimming pool with spa for 3 people and sea view")

        Nls.package_update(package)

      end



      def test_ltras_single_words

        sentence = "with a nice swimmming pol with spa for 3 peple and sea viiaw"

        check_interpret(sentence, interpretation: "ltras", score: 0.95)

      end
    end
  end
end
