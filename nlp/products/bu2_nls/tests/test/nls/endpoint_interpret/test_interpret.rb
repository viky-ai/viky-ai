# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < NlsTestCommon

      def test_interpret_simple

        several_packages_several_intents

        Nls.restart

        package = available_packages['datetime2']

        interpretation = package.interpretation('hello1')
        sentence = 'Hello Nicolas'

        actual = Nls.interpret_package(package, sentence)

        expected = {
          "interpretations" => [ interpretation.to_match  ]
        }

        assert_equal expected, actual

      end

      def test_interpret_several_package_same_sentence

        several_packages_several_intents

        Nls.restart

        package_1 = available_packages['datetime1']
        package_2 = available_packages['datetime3']
        package_3 = available_packages['samesentence1']
        package_4 = available_packages['samesentence2']

        sentence = "Hello Sebastien"

        json_interpretation_1 = package_1.interpretation('hello3')
        json_interpretation_2 = package_2.interpretation('hello2')
        json_interpretation_3 = package_3.interpretation('hello4')
        json_interpretation_4 = package_4.interpretation('hello5')

        param =
        {
          "packages" => [package_1.id, package_2.id, package_3.id, package_4.id],
          "sentence" => sentence,
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "interpretations" =>
          [
            json_interpretation_4.to_match,
            json_interpretation_3.to_match,
            json_interpretation_2.to_match,
            json_interpretation_1.to_match
          ]
        }

        assert_equal expected, actual

      end

      def test_interpret_parallel_query

        interpretations = []

        # Prepare data
        package = Package.new("package_test_interpret_parallel_query")
        1000.times do |j|
          interpretation = Interpretation.new("interpretation_#{j}")
          interpretation << Expression.new("sentence_#{j}")
          interpretations << interpretation
          package << interpretation
        end
        package.to_file(importDir)

        Nls.restart

        # Run parallel queries
        Parallel.map(interpretations, in_threads: 20) do |interpretation|

          package = interpretation.package
          sentence = interpretation.expressions.first.expression

          actual = Nls.interpret_package(package, sentence)

          expected =
          {
            "interpretations" => [ interpretation.to_match ]
          }

          assert_equal expected, actual

        end
      end

      def test_empty_sentence

        several_packages_several_intents

        Nls.restart

        package = available_packages['datetime1']
        expected_error = "NlsCheckRequestString : empty text in request"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret_package(package, "")
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_too_long_sentence

        several_packages_several_intents

        Nls.restart

        package = available_packages['datetime1']

        sentence = "abcdefghij"
        210.times do
          sentence << "abcdefghij"
        end

        expected_error = "NlsCheckRequestString : too long text in request"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret_package(package, sentence)
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_special_char

        cp_import_fixture("package_with_special_char.json")

        Nls.restart

        json_query_special_char = JSON.parse(File.read(fixture_path("query_with_special_char.json")))

        actual = Nls.interpret(json_query_special_char)

        expected =
        {
          "interpretations" =>
          [
          {
          "package" => "voqal.ai:datetime",
          "id" => "0d981484-9313-11e7-abc4-cec278b6b50b",
          "slug" => "special_char",
          "score" => 1.0
          }
          ]
        }

        assert_equal expected, actual
      end

      def test_no_sentence_match

        several_packages_several_intents

        Nls.restart

        package = available_packages['datetime1']
        actual = Nls.interpret_package(package, "azerty")

        expected =
        {
          "interpretations" => []
        }

        assert_equal expected, actual

      end

    end

  end

end
