# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < Common

      def test_interpret_simple

        json_structure = Nls.several_packages_several_intents
        generate_multiple_package_file(json_structure)

        Nls.restart

        package_id = json_structure[0]["id"]
        expression = json_structure[0]["interpretations"][0]["expressions"][1]["expression"]
        interpretation_id = json_structure[0]["interpretations"][0]["id"]
        slug = json_structure[0]["interpretations"][0]["slug"]


        actual = Nls.interpret(Nls.json_interpret_body(package_id, expression))
        expected_interpret_result = Nls.expected_interpret_result(package_id, interpretation_id, slug)

        assert_equal expected_interpret_result, actual

      end

      def test_interpret_several_package_same_sentence

        json_structure = Nls.several_packages_same_sentence(10)
        write_json_to_import_dir(json_structure, "several_packages_same_sentence.json")

        Nls.restart

        param =
        {
          "packages" => [json_structure[1]["id"],json_structure[2]["id"],json_structure[0]["id"],json_structure[8]["id"]],
          "sentence" => json_structure[1]["interpretations"][7]["expressions"][0]["expression"],
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "interpretations" =>
          [
          {
          "package" => json_structure[1]["id"],
          "id" => json_structure[1]["interpretations"][7]["id"],
          "slug" => json_structure[1]["interpretations"][7]["slug"],
          "score" => 1.0
          },
          {
          "package" => json_structure[2]["id"],
          "id" => json_structure[2]["interpretations"][7]["id"],
          "slug" => json_structure[2]["interpretations"][7]["slug"],
          "score" => 1.0
          },
          {
          "package" => json_structure[0]["id"],
          "id" => json_structure[0]["interpretations"][7]["id"],
          "slug" => json_structure[0]["interpretations"][7]["slug"],
          "score" => 1.0
          },
          {
          "package" => json_structure[8]["id"],
          "id" => json_structure[8]["interpretations"][7]["id"],
          "slug" => json_structure[8]["interpretations"][7]["slug"],
          "score" => 1.0
          }
          ]
        }

        assert_equal expected, actual

      end

      def test_interpret_parallel_query

        json_structure = Nls.createHugePackagesFile(10)
        write_json_to_import_dir(json_structure, "several_packages_parallelize.json")

        Nls.restart

        expressions_array = [];

        for package in 1..10
          for interpretation in 1..10
            for expression in 1..10
              expressions_array << [json_structure[package-1]["id"],
                                    json_structure[package-1]["interpretations"][interpretation-1]["id"],
                                    json_structure[package-1]["interpretations"][interpretation-1]["slug"],
                                    json_structure[package-1]["interpretations"][interpretation-1]["expressions"][expression-1]["expression"]]
            end
          end
        end

        Parallel.map(expressions_array, in_threads: 20) do |expression_array|

          param =
          {
            "packages" => [expression_array[0]],
            "sentence" => expression_array[3],
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
            "slug" => expression_array[2],
            "score" => 1.0
            }
            ]
          }

          assert_equal expected, actual

        end
      end

      def test_empty_sentence

        json_structure = Nls.package_without_error
        write_json_to_import_dir(json_structure, "package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => [json_structure[0]["id"]],
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

        json_structure = Nls.package_without_error
        write_json_to_import_dir(json_structure, "package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => [json_structure[0]["id"]],
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

        json_structure = Nls.package_without_error
        write_json_to_import_dir(json_structure, "package_without_error.json")

        Nls.restart

        param =
        {
          "packages" => [json_structure[0]["id"]],
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
