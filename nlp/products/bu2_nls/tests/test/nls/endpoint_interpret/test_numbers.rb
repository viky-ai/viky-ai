# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr-FR"

        Nls.package_update(create_numbers_package)
      end

      def test_simple_decimal_number_french
        check_interpret("1",              { interpretation: "numbers_list", solution: {numbers: [1]} })
        check_interpret("1,2",            { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("12 345 678",     { interpretation: "numbers_list", solution: {numbers: [12345678]} })
      end

      def test_simple_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1.2",            { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("12,345,678",     { interpretation: "numbers_list", solution: {numbers: [12345678]} })
      end

      def test_extra_zero_french
        check_interpret("01,2",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,20",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,200",   { interpretation: "numbers_list", solution: {numbers: [1.2]} })
      end

      def test_extra_zero_english
        Interpretation.default_locale = "en-US"
        check_interpret("01.2",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1.20",    { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1.200",   { interpretation: "numbers_list", solution: {numbers: [1.2]} })
      end

      def test_thousand_number_french
        check_interpret("1 200",      { interpretation: "numbers_list", solution: {numbers: [1200]} })
        check_interpret("100 200",    { interpretation: "numbers_list", solution: {numbers: [100200]} })
        check_interpret("9 100 200",  { interpretation: "numbers_list", solution: {numbers: [9100200]} })
        check_interpret("1 234",      { interpretation: "numbers_list", solution: {numbers: [1234]} })
        check_interpret("1 234 567",  { interpretation: "numbers_list", solution: {numbers: [1234567]} })
      end

      def test_thousand_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1,200",      { interpretation: "numbers_list", solution: {numbers: [1200]} })
        check_interpret("100,200",    { interpretation: "numbers_list", solution: {numbers: [100200]} })
        check_interpret("9,100,200",  { interpretation: "numbers_list", solution: {numbers: [9100200]} })
        check_interpret("1,234",      { interpretation: "numbers_list", solution: {numbers: [1234]} })
        check_interpret("1,234,567",  { interpretation: "numbers_list", solution: {numbers: [1234567]} })
      end

      def test_thousand_decimal_number_french
        check_interpret("1 200,3",      { interpretation: "numbers_list", solution: {numbers: [1200.3]} })
        check_interpret("9 100 200,0",  { interpretation: "numbers_list", solution: {numbers: [9100200]} })
        check_interpret("9100200,0",    { interpretation: "numbers_list", solution: {numbers: [9100200]} })
      end

      def test_thousand_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1,200.3",      { interpretation: "numbers_list", solution: {numbers: [1200.3]} })
        check_interpret("9,100,200.0",  { interpretation: "numbers_list", solution: {numbers: [9100200]} })
        check_interpret("9100200.0",    { interpretation: "numbers_list", solution: {numbers: [9100200]} })
      end

      def test_complex_number
        check_interpret("a 1 b",              { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1, "letter2"=>"b"} })
        check_interpret("a 1,2 b",            { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1.2, "letter2"=>"b"} })
        check_interpret("12 123,123 123",     { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("12 123,123,123",     { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
      end

      def test_wrong_decimal_number_french
        check_interpret("1 , 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1, 2",     { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 ,2",     { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1.2",      { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
      end

      def test_wrong_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1 . 2",    { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1. 2",     { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 .2",     { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1,2",      { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
      end

      def test_wrong_thousand_number_french
        check_interpret("9 100 200 0",    { interpretation: "numbers_list", solution: {numbers: [9100200, 0]} })
        check_interpret("33 12 34 56 78", { interpretation: "numbers_list", solution: {numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234 123",       { interpretation: "numbers_list", solution: {numbers: [1234, 123]} })
        check_interpret("12 123,123 123", { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("1, 2, 3,4",      { interpretation: "numbers_list", solution: {numbers: [1, 2, 3.4]} })
      end

      def test_wrong_thousand_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("9,100,200,0",    { interpretation: "numbers_list", solution: {numbers: [9100200, 0]} })
        check_interpret("33,12,34,56,78", { interpretation: "numbers_list", solution: {numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234,123",       { interpretation: "numbers_list", solution: {numbers: [1234, 123]} })
        check_interpret("12,123.123,123", { interpretation: "numbers_list", solution: {numbers: [12123.123, 123]} })
        check_interpret("12,123,12 123",    { interpretation: "numbers_list", solution: {numbers: [12123, 12, 123]} })
        check_interpret("1, 2, 3.4",      { interpretation: "numbers_list", solution: {numbers: [1, 2, 3.4]} })
      end

      def test_combinations
        check_interpret("123 titi",                   { interpretation: "combinations", solution: { combination: [{number: 123}] } })
        check_interpret("123 toto",                   { interpretation: "combinations", solution: { combination: [{number: 123}] } })
        check_interpret("123 titi 456 toto",          { interpretation: "combinations", solution: { combination: [{number: 123}, {number: 456}] } })
        check_interpret("123 titi 456 toto 789 titi", { interpretation: "combinations", solution: { combination: [{number: 123}, {number: 456}, {number: 789}] } })
      end

      def test_number_with_text_pasted
        check_interpret("123st 456",    { interpretation: "numbers_list", solution: { numbers: [123, 456] } })
      end

      def test_multilang
        Interpretation.default_locale = "fr-FR,en-US"
        check_interpret("1.2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })

        Interpretation.default_locale = nil
        check_interpret("1.2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })

        Interpretation.default_locale = "fr-FR"
        check_interpret("1.2",  { interpretation: "numbers_list", solution: {numbers: [1, 2]} })
        check_interpret("1,2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })

        Interpretation.default_locale = "en-US"
        check_interpret("1.2",  { interpretation: "numbers_list", solution: {numbers: [1.2]} })
        check_interpret("1,2",  { interpretation: "numbers_list", solution: {numbers: [1, 2]} })

      end

    end
  end

end
