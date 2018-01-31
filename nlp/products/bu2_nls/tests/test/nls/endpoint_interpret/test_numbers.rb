# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr-FR"

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("numbers")

        letters = package.new_interpretation("letters")
        letters << Expression.new("a", solution: "a")
        letters << Expression.new("b", solution: "b")

        numbers = package.new_interpretation("numbers")
        numbers << Expression.new("@{number}", aliases: { number: Alias.digit })

#        titi = package.new_interpretation("titi")
#        titi << Expression.new("titi")
#
#        toto = package.new_interpretation("toto")
#        toto << Expression.new("toto")
#
#        combination1 = package.new_interpretation("combination1")
#        combination1 << Expression.new("@{number} @{toto}", aliases: { number: Alias.digit, toto: toto })
#
#        combination2 = package.new_interpretation("combination2")
#        combination2 << Expression.new("@{number} @{titi}", aliases: { number: Alias.digit, titi: titi })
#
#        combination = package.new_interpretation("combination")
#        combination << Expression.new("@{combination2}", aliases: { combination2: combination2 })
#        combination << Expression.new("@{combination1}", aliases: { combination1: combination1 })
#
#        combinations = package.new_interpretation("combinations")
#        combinations << Expression.new(" @{combination} @{combinations}", aliases: {combination: combination, combinations: combinations})
#        combinations << Expression.new("@{combination}", aliases: {combination: combination})


        numbers_list = package.new_interpretation("numbers_list")
        numbers_list << Expression.new("@{numbers_list} @{numbers}", aliases: {numbers_list: numbers_list, numbers: numbers})
        numbers_list << Expression.new("@{numbers}", aliases: {numbers: numbers})

        complex_number = package.new_interpretation("complex_number")
        complex_number << Expression.new("@{letter1} @{number} @{letter2}", aliases: { letter1: letters, number: Alias.digit, letter2: letters})

        package
      end

      # Tests

      def test_simple_decimal_number_french
        check_interpret("1",      { interpretation: "numbers_list", solution: {numbers: [1]} })
        check_interpret("1,2",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("12 345 678",    { interpretation: "numbers_list", solution: {numbers: [12345678]} })
      end

      def test_extra_zero
        check_interpret("01,2",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,20",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,200",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
      end

      def test_thousand_number
        check_interpret("1 200",    { interpretation: "numbers_list", solution: {numbers: [1200]} })
        check_interpret("100 200",    { interpretation: "numbers_list", solution: {numbers: [100200]} })
        check_interpret("9 100 200",    { interpretation: "numbers_list", solution: {numbers: [9100200]} })

        check_interpret("1 234",    { interpretation: "numbers_list", solution: {numbers: [1234]} })
        check_interpret("1 234 567",    { interpretation: "numbers_list", solution: {numbers: [1234567]} })
      end

      def test_thousand_decimal_number
        check_interpret("1 200,3",    { interpretation: "numbers_list", solution: {numbers: [1200.3]} })
        check_interpret("9 100 200,0",    { interpretation: "numbers_list", solution: {numbers: [9100200]} })
        check_interpret("9100200,0",    { interpretation: "numbers_list", solution: {numbers: [9100200]} })
      end

      def test_complex_number
        check_interpret("a 1 b",      { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1, "letter2"=>"b"} })
        check_interpret("a 1,2 b",    { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1.2, "letter2"=>"b"} })
        check_interpret("12 123,123 123",    { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("12 123,123,123",    { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
      end

      def test_wrong_decimal_number_french
        check_interpret("1 , 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1, 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 ,2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
      end

      def test_wrong_thousand_number_french
        check_interpret("9 100 200 0",    { interpretation: "numbers_list", solution: {numbers: [9100200, 0]} })
        check_interpret("33 12 34 56 78",    { interpretation: "numbers_list", solution: {numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234 123",    { interpretation: "numbers_list", solution: {numbers: [1234, 123]} })
        check_interpret("12 123,123 123",    { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("1, 2, 3,4",    { interpretation: "numbers_list", solution: {numbers: [1, 2, 3.4]} })
      end

      def test_other_languages
        skip "other languages are not working yet"

        Interpretation.default_locale = "en-US"
        check_interpret("1.2",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("12,345,678",    { interpretation: "numbers_list", solution: {numbers: [12345678]} })

        check_interpret("1 . 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1. 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 .2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("9,100,200,0",    { interpretation: "numbers_list", solution: {numbers: [9100200, 0]} })
        check_interpret("12,123,123 123",    { interpretation: "numbers_list", solution: {numbers: [12123123, 123]} })
        check_interpret("12,123,12 123",    { interpretation: "numbers_list", solution: {numbers: [12123, 12, 123]} })
        check_interpret("12,123.123 123",    { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("12 123.123 123",    { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("1, 2, 3.4",    { interpretation: "numbers_list", solution: {numbers: [1, 2, 3.4]} })

      end

#      def test_combinations
##        check_interpret("123 titi",    { interpretation: "combinations", solution: {numbers: [1.2]} })
#        check_interpret("123 toto",    { interpretation: "combinations", solution: {numbers: [123]} })
#      end

    end
  end

end
