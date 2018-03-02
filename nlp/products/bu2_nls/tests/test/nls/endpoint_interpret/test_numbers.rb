# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      # Rappel de la conf effective :
      #
      # { thousands: " ", decimal: "," }  # 1 234,56   -- French
      # { thousands: ",", decimal: "." }  # 1,234.56   -- United States
      # { thousands: ".", decimal: "," }  # 1.234,56   -- German
      # { thousands: "'", decimal: "." }  # 1'234.56   -- Switzerland
      #

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

        Nls.package_update(create_numbers_package)
      end

      def test_simple_decimal_number_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("1",              { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1,2",            { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("12 345 678",     { interpretation: "numbers_list", solution: { numbers: [12345678]} })
      end

      def test_simple_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1.2",            { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("12,345,678",     { interpretation: "numbers_list", solution: { numbers: [12345678]} })
      end

      def test_simple_decimal_number_german
        Interpretation.default_locale = "de-DE"
        check_interpret("1,2",            { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("12.345.678",     { interpretation: "numbers_list", solution: { numbers: [12345678]} })
      end

      def test_simple_decimal_number_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("1.2",            { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("12'345'678",     { interpretation: "numbers_list", solution: { numbers: [12345678]} })
      end

      def test_extra_zero_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("01,2",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1,20",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1,200",   { interpretation: "numbers_list", solution: { numbers: [1.2]} })
      end

      def test_extra_zero_english
        Interpretation.default_locale = "en-US"
        check_interpret("01.2",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.20",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.200",   { interpretation: "numbers_list", solution: { numbers: [1.2]} })
      end

      def test_extra_zero_german
        Interpretation.default_locale = "de-DE"
        check_interpret("01,2",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1,20",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1,200",   { interpretation: "numbers_list", solution: { numbers: [1.2]} })
      end

      def test_extra_zero_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("01.2",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.20",    { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.200",   { interpretation: "numbers_list", solution: { numbers: [1.2]} })
      end

      def test_thousand_number_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("1 200",      { interpretation: "numbers_list", solution: { numbers: [1200]} })
        check_interpret("100 200",    { interpretation: "numbers_list", solution: { numbers: [100200]} })
        check_interpret("9 100 200",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("1 234",      { interpretation: "numbers_list", solution: { numbers: [1234]} })
        check_interpret("1 234 567",  { interpretation: "numbers_list", solution: { numbers: [1234567]} })
      end

      def test_thousand_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1,200",      { interpretation: "numbers_list", solution: { numbers: [1200]} })
        check_interpret("100,200",    { interpretation: "numbers_list", solution: { numbers: [100200]} })
        check_interpret("9,100,200",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("1,234",      { interpretation: "numbers_list", solution: { numbers: [1234]} })
        check_interpret("1,234,567",  { interpretation: "numbers_list", solution: { numbers: [1234567]} })
      end

      def test_thousand_number_german
        Interpretation.default_locale = "de-DE"
        check_interpret("1.200",      { interpretation: "numbers_list", solution: { numbers: [1200]} })
        check_interpret("100.200",    { interpretation: "numbers_list", solution: { numbers: [100200]} })
        check_interpret("9.100.200",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("1.234",      { interpretation: "numbers_list", solution: { numbers: [1234]} })
        check_interpret("1.234.567",  { interpretation: "numbers_list", solution: { numbers: [1234567]} })
      end

      def test_thousand_number_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("1'200",      { interpretation: "numbers_list", solution: { numbers: [1200]} })
        check_interpret("100'200",    { interpretation: "numbers_list", solution: { numbers: [100200]} })
        check_interpret("9'100'200",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("1'234",      { interpretation: "numbers_list", solution: { numbers: [1234]} })
        check_interpret("1'234'567",  { interpretation: "numbers_list", solution: { numbers: [1234567]} })
      end

      def test_thousand_decimal_number_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("1 200,3",      { interpretation: "numbers_list", solution: { numbers: [1200.3]} })
        check_interpret("9 100 200,0",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("9100200,0",    { interpretation: "numbers_list", solution: { numbers: [9100200]} })
      end

      def test_thousand_decimal_number_french_with_score
        Interpretation.default_locale = "fr-FR"
        check_interpret("1 200,3",      { interpretation: "numbers_list", solution: { numbers: [1200.3]}, score: 1.0 })
        check_interpret("9 100 200,0",  { interpretation: "numbers_list", solution: { numbers: [9100200]}, score: 1.0 })
        check_interpret("9100200,0",    { interpretation: "numbers_list", solution: { numbers: [9100200]}, score: 1.0 })
      end

      def test_thousand_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1,200.3",      { interpretation: "numbers_list", solution: { numbers: [1200.3]} })
        check_interpret("9,100,200.0",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("9100200.0",    { interpretation: "numbers_list", solution: { numbers: [9100200]} })
      end

      def test_thousand_decimal_number_german
        Interpretation.default_locale = "de-DE"
        check_interpret("1.200,3",      { interpretation: "numbers_list", solution: { numbers: [1200.3]} })
        check_interpret("9.100.200,0",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("9100200,0",    { interpretation: "numbers_list", solution: { numbers: [9100200]} })
      end

      def test_thousand_decimal_number_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("1'200.3",      { interpretation: "numbers_list", solution: { numbers: [1200.3]} })
        check_interpret("9'100'200.0",  { interpretation: "numbers_list", solution: { numbers: [9100200]} })
        check_interpret("9100200.0",    { interpretation: "numbers_list", solution: { numbers: [9100200]} })
      end

      def test_complex_number
        Interpretation.default_locale = "fr-FR"
        check_interpret("a 1 b",              { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1, "letter2"=>"b"} })
        check_interpret("a 1,2 b",            { interpretation: "complex_number", solution: {"letter1"=>"a", "number"=>1.2, "letter2"=>"b"} })
        check_interpret("12 123,123 123",     { interpretation: "numbers_list", solution: { numbers: [12123.123, 123]} })
        check_interpret("12 123,123,123",     { interpretation: "numbers_list", solution: { numbers: [12123.123, 123]} })
      end

      def test_wrong_decimal_number_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("1 , 2",    { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1, 2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 ,2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
      end

      def test_wrong_decimal_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("1 . 2",    { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1. 2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 .2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1,2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
      end

      def test_wrong_decimal_number_german
        Interpretation.default_locale = "de-DE"
        check_interpret("1 , 2",    { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1, 2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 ,2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
      end

      def test_wrong_decimal_number_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("1 . 2",    { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1. 2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 .2",     { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1 2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1,2",      { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
      end

      def test_wrong_thousand_number_french
        Interpretation.default_locale = "fr-FR"
        check_interpret("9 100 200 0",    { interpretation: "numbers_list", solution: { numbers: [9100200, 0]} })
        check_interpret("33 12 34 56 78", { interpretation: "numbers_list", solution: { numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234 123",       { interpretation: "numbers_list", solution: { numbers: [1234, 123]} })
        check_interpret("12 123,123 123", { interpretation: "numbers_list", solution: { numbers: [12123.123, 123]} })
        check_interpret("1, 2, 3,4",      { interpretation: "numbers_list", solution: { numbers: [1, 2, 3.4]} })
      end

      def test_wrong_thousand_number_english
        Interpretation.default_locale = "en-US"
        check_interpret("9,100,200,0",    { interpretation: "numbers_list", solution: { numbers: [9, 100, 200, 0]} })
        check_interpret("33,12,34,56,78", { interpretation: "numbers_list", solution: { numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234,123",       { interpretation: "numbers_list", solution: { numbers: [1234, 123]} })
        check_interpret("12,123.123,123", { interpretation: "numbers_list", solution: { numbers: [12, 123, 123, 123]} })
        check_interpret("12,123,12 123",  { interpretation: "numbers_list", solution: { numbers: [12123, 12, 123]} })
        check_interpret("1, 2, 3.4",      { interpretation: "numbers_list", solution: { numbers: [1, 2, 3.4]} })
      end

      def test_wrong_thousand_number_german
        Interpretation.default_locale = "de-DE"
        check_interpret("9.100.200.0",    { interpretation: "numbers_list", solution: { numbers: [9, 100, 200, 0]} })
        check_interpret("33.12.34.56.78", { interpretation: "numbers_list", solution: { numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234.123",       { interpretation: "numbers_list", solution: { numbers: [1234, 123]} })
        check_interpret("12.123,123.123", { interpretation: "numbers_list", solution: { numbers: [12, 123, 123, 123]} })
        check_interpret("12.123.12 123",  { interpretation: "numbers_list", solution: { numbers: [12123, 12, 123]} })
        check_interpret("1. 2. 3,4",      { interpretation: "numbers_list", solution: { numbers: [1, 2, 3.4]} })
      end

      def test_wrong_thousand_number_swiss
        Interpretation.default_locale = "fr-CH"
        check_interpret("9'100'200'0",    { interpretation: "numbers_list", solution: { numbers: [9, 100, 200, 0]} })
        check_interpret("33'12'34'56'78", { interpretation: "numbers_list", solution: { numbers: [33, 12, 34, 56, 78]} })
        check_interpret("1234'123",       { interpretation: "numbers_list", solution: { numbers: [1234, 123]} })
        check_interpret("12'123.123'123", { interpretation: "numbers_list", solution: { numbers: [12, 123, 123, 123]} })
        check_interpret("12'123,12 123",  { interpretation: "numbers_list", solution: { numbers: [12123, 12, 123]} })
        check_interpret("1' 2' 3.4",      { interpretation: "numbers_list", solution: { numbers: [1, 2, 3.4]} })
      end

      def test_combinations
        check_interpret("123 titi",                   { interpretation: "combinations", solution: { combination: [{number: 123}] } })
        check_interpret("123 toto",                   { interpretation: "combinations", solution: { combination: [{number: 123}] } })
        check_interpret("123 titi 456 toto",          { interpretation: "combinations", solution: { combination: [{number: 123}, {number: 456}] } })
        check_interpret("123 titi 456 toto 789 titi", { interpretation: "combinations", solution: { combination: [{number: 123}, {number: 456}, {number: 789}] } })
      end

      def test_number_with_text_pasted
        check_interpret("abc123st 456",    { interpretation: "numbers_list", solution: { numbers: [123, 456] } })
      end

      def test_multilang_decimal
        Interpretation.default_locale = "fr-FR,en-US,en-UK,en,de-CH,de"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = nil
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = "fr-FR"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })

        Interpretation.default_locale = "fr-CA"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })

        Interpretation.default_locale = "fr"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })

        Interpretation.default_locale = "en-US"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = "en-UK"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = "en"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = "de-DE"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })

        Interpretation.default_locale = "de"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })

        Interpretation.default_locale = "fr-CH"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

        Interpretation.default_locale = "de-CH"
        check_interpret("1,2", { interpretation: "numbers_list", solution: { numbers: [1, 2]} })
        check_interpret("1.2", { interpretation: "numbers_list", solution: { numbers: [1.2]} })

      end

      def test_multilang_thousand


        Interpretation.default_locale = "fr-FR,en-US,en-UK,en,de-CH,de"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = nil
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "*"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "fr-FR"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })

        Interpretation.default_locale = "fr-CA"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })

        Interpretation.default_locale = "fr"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })

        Interpretation.default_locale = "en-US"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "en-UK"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "en"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "de-DE"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1000]} })

        Interpretation.default_locale = "de"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1000]} })

        Interpretation.default_locale = "fr-CH"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

        Interpretation.default_locale = "de-CH"
        check_interpret("1 000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1,000", { interpretation: "numbers_list", solution: { numbers: [1, 0]} })
        check_interpret("1'000", { interpretation: "numbers_list", solution: { numbers: [1000]} })
        check_interpret("1.000", { interpretation: "numbers_list", solution: { numbers: [1]} })

      end

      def test_multilang_thousand_and_decimal

       Interpretation.default_locale = "fr-FR,en-US,en-UK,en,de-CH,de"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })

       Interpretation.default_locale = nil
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })

       Interpretation.default_locale = "fr-FR"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "fr-CA"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "fr"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "en-US"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "en-UK"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "en"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "de-DE"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })

       Interpretation.default_locale = "de"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0.2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })

       Interpretation.default_locale = "fr-CH"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

       Interpretation.default_locale = "de-CH"
       check_interpret("1 000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1,000.2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })
       check_interpret("1'000.2", { interpretation: "numbers_list", solution: { numbers: [1000.2]} })
       check_interpret("1.000,2", { interpretation: "numbers_list", solution: { numbers: [1, 0, 2]} })

      end


      def test_number_in_string
        skip "#16960: TODO : à corriger"
        package = Package.new("test_number_in_string")

        interpretation_private = package.new_interpretation("element")
        interpretation_private << Expression.new("for 3 days")

        Nls.remove_all_packages

        Interpretation.default_locale = nil

        Nls.package_update(package)

        check_interpret("for 3 days",
          interpretation: "element",
          solution: "for 3 days"
        )

      end


    end
  end

end
