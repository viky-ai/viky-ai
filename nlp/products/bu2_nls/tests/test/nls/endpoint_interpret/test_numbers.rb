# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("numbers")

        letters = package.new_interpretation("letters")
        letters << Expression.new("a", solution: "a")
        letters << Expression.new("b", solution: "b")

        numbers = package.new_interpretation("numbers")
        numbers << Expression.new("@{number}", aliases: { number: Alias.digit })

#        numbers_list = package.new_interpretation("numbers_list")
#        numbers_list << Expression.new("@{numbers_list} @{numbers}", aliases: {numbers_list: numbers_list, numbers: numbers})
#        numbers_list << Expression.new("@{numbers}", aliases: {numbers: numbers})

        complex_number = package.new_interpretation("complex_number")
        complex_number << Expression.new("@{letter1} @{number} @{letter2}", aliases: { letter1: letters, number: Alias.digit, letter2: letters})

        package
      end

      # Tests

      def test_simple_decimal_number_french
        check_interpret("1",      { interpretation: "numbers", solution: 1 })
        check_interpret("1,2",    { interpretation: "numbers", solution: 1.2 })
        check_interpret("12 345 678",    { interpretation: "numbers", solution: 12345678 })
#        check_interpret("1",      { interpretation: "numbers", solution: 1 })
#        check_interpret("1,2",    { interpretation: "numbers", solution: 1.2 })
#        check_interpret("12 345 678",    { interpretation: "numbers", solution: 12345678 })
      end

      def test_wrong_decimal_french
        check_interpret("1 , 2",    { interpretation: "numbers", solution: 1 })
        check_interpret("1, 2",    { interpretation: "numbers", solution: 1 })
        check_interpret("1 ,2",    { interpretation: "numbers", solution: 1 })
      end

      def test_extra_zero
        check_interpret("01,2",    { interpretation: "numbers", solution: 1.2 })
        check_interpret("1,20",    { interpretation: "numbers", solution: 1.2 })
        check_interpret("1,200",    { interpretation: "numbers", solution: 1.2 })
      end


      def test_thousand_number
        check_interpret("1 200",    { interpretation: "numbers", solution: 1200 })
        check_interpret("100 200",    { interpretation: "numbers", solution: 100200 })
        check_interpret("9 100 200",    { interpretation: "numbers", solution: 9100200 })

        check_interpret("1 234",    { interpretation: "numbers", solution: 1234 })
        check_interpret("1 234 567",    { interpretation: "numbers", solution: 1234567 })
      end

      def test_thousand_decimal_number
        check_interpret("1 200,3",    { interpretation: "numbers", solution: 1200.3 })
        check_interpret("9 100 200.0",    { interpretation: "numbers", solution: 9100200 })
      end



      def test_complex_number
        check_interpret("a 1 b",      { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1, "letter2"=>"b"} })
        check_interpret("a 1,2 b",    { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1.2, "letter2"=>"b"} })
        check_interpret("12 123,123 123",    { interpretation: "numbers", solution: 12123.123 })
        check_interpret("12 123,123 123",    { interpretation: "numbers", solution: 12123.123 })
      end

    end
  end

end
