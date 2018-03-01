# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLtras < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil


        ltras_package = Package.new("ltras")
        ltras = ltras_package.new_interpretation("ltras")
        ltras << Expression.new("swimming pool with spa for 3 people and sea view")

        Nls.package_update(ltras_package)

        parse_package = Package.new("parse")
        ltras = parse_package.new_interpretation("parse")
        ltras << Expression.new("abc123def,i:jklm456opq;")

        Nls.package_update(parse_package)

      end



      def test_ltras_single_words

        sentence = "with a nice swimmming pol with spa for 3 peple and sea viiaw"
        check_interpret(sentence, interpretation: "ltras", score: 0.93)

      end

      def test_complexe_parsing

        sentence = "abc 123 def , i : jklm 456 opq ;"
        check_interpret(sentence, interpretation: "parse", score: 1.0)

      end
    end
  end
end
