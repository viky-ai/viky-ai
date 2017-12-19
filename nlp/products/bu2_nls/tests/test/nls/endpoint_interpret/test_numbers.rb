# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr-FR"

        Nls.package_update(fixture_parse("package_number_digits.json"))
        Nls.package_update(fixture_parse("package_number_letters.json"))
        Nls.package_update(fixture_parse("package_number.json"))
      end

      def test_simple_numbers
        check_interpret("3", interpretation: "number", solution: { number: 3 } )
        check_interpret("4", interpretation: "number", solution: { number: 4 } )
      end

      def test_simple_number_letters
        check_interpret("three", interpretation: "number", solution: { number: 3 } )
        check_interpret("quatre", interpretation: "number", solution: { number: 4 } )

      end

      def test_complex_number
        check_interpret("51", interpretation: "number", solution: { number: 51 } )
        check_interpret("32847", interpretation: "number", solution: { number: 32847 } )
      end

      def test_complex_number_letters
        check_interpret("fifty two", interpretation: "number", solution: { number: 52 } )
        check_interpret("two hundred and fifty two", interpretation: "number", solution: { number: 252 } )
        check_interpret("quarante et un", interpretation: "number", solution: { number: 41 } )
        check_interpret("quarante et une", interpretation: "number", solution: { number: 41 } )
        check_interpret("quatre vingt douze", interpretation: "number", solution: { number: 92 } )
        check_interpret("sept cent dix huit mille quatre cent quatre vingt quatorze", interpretation: "number", solution: { number: 718494 } )
        check_interpret("quarante douze", interpretation: "number", solution: { number: 52 } )
        check_interpret("two thousand and seventeen", interpretation: "number", solution: { number: 2017 } )
      end

      def test_ordinal_number
        check_interpret("troisieme", interpretation: "number_ordinal", solution: { number: 3 } )
        check_interpret("third", interpretation: "number_ordinal", solution: { number: 3 } )
        check_interpret("3 rd", interpretation: "number_ordinal", solution: { number: 3 } )
        check_interpret("1 st", interpretation: "number_ordinal", solution: { number: 1 } )
        check_interpret("4 eme", interpretation: "number_ordinal", solution: { number: 4 } )
        check_interpret("236 th", interpretation: "number_ordinal", solution: { number: 236 } )
        check_interpret("236 ieme", interpretation: "number_ordinal", solution: { number: 236 } )
        check_interpret("centième", interpretation: "number_ordinal", solution: { number: 100 } )

#        check_interpret("3ieme", interpretation: "number_ordinal", solution: { number: 3 } )
#        check_interpret("3rd", interpretation: "number_ordinal", solution: { number: 3 } )
      end
    end
  end
end
