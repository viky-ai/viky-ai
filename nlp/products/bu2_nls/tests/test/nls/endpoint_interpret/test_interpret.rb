# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < Common

      def test_interpret_simple

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        actual = Nls.interpret(Nls.json_interpret_body)

        assert_equal Nls.expected_interpret_result, actual

      end

      def test_interpret_several_package_same_sentence

        cp_import_fixture("several_packages_same_sentence.json")

        Nls.restart

        param =
        {
          "packages" => ["package_2","package_3","package_1","package_9"],
          "sentence" => "expression_8",
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "interpretations" =>
          [
          {
          "package" => "package_2",
          "id" => "intent_2_8",
          "slug" => "intent_2_8",
          "score" => 1.0
          },
          {
          "package" => "package_3",
          "id" => "intent_3_8",
          "slug" => "intent_3_8",
          "score" => 1.0
          },
          {
          "package" => "package_1",
          "id" => "intent_1_8",
          "slug" => "intent_1_8",
          "score" => 1.0
          },
          {
          "package" => "package_9",
          "id" => "intent_9_8",
          "slug" => "intent_9_8",
          "score" => 1.0
          }
          ]
        }

        assert_equal expected, actual

      end

      def test_interpret_parallel_query

        createHugePackagesFile("several_packages_parallelize.json", 10)

        Nls.restart

        expressions_array = [];

        for package in 1..10
          for interpretation in 1..10
            for expression in 1..10
              expressions_array << ["package_#{package}","interpretation_#{package}_#{interpretation}","expression_#{package}_#{interpretation}_#{expression}"]
            end
          end
        end

        Parallel.map(expressions_array, in_threads: 20) do |expression_array|

          param =
          {
            "packages" => [expression_array[0]],
            "sentence" => expression_array[2],
            "Accept-Language" => "fr-FR"
          }

          actual = Nls.interpret(param)

          expected =
          {
            "interpretations" =>
            [
            {
            "package" => expression_array[0],
            "id" => expression_array[1],
            "slug" => expression_array[1],
            "score" => 1.0
            }
            ]
          }

          assert_equal expected, actual

        end
      end

      def test_empty_sentence

        cp_import_fixture("package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => ["voqal.ai:datetime1"],
          "sentence" => "",
          "Accept-Language" => "fr-FR"
        }

        expected_error = "NlsCheckRequestString : empty text in request"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret(param)
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_too_long_sentence

        cp_import_fixture("package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => ["voqal.ai:datetime"],
          "sentence" => "abcdefghij",
          "Accept-Language" => "fr-FR"
        }

        210.times do
          param["sentence"] << "abcdefghij"
        end

        expected_error = "NlsCheckRequestString : too long text in request"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret(param)
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

        cp_import_fixture("package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => ["voqal.ai:datetime"],
          "sentence" => "azerty",
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "interpretations" => []
        }

        assert_equal expected, actual

      end

    end

  end

end
