# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < NlsTestCommon

      def test_interpret_simple
        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

        package = available_packages['datetime2']

        expected = Answers.new(package.interpretation('hello1'))
        sentence = 'Hello Nicolas'

        actual = Nls.interpret_package(package, sentence)

        assert_json expected.to_h, actual

      end

      def test_interpret_several_package_same_sentence

        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

        package_1 = available_packages['datetime1']
        package_2 = available_packages['datetime3']
        package_3 = available_packages['samesentence1']
        package_4 = available_packages['samesentence2']

        sentence = "Hello Sebastien"

        param =
        {
          packages: [package_1.id.to_s, package_2.id.to_s, package_3.id.to_s, package_4.id.to_s],
          sentence: sentence,
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected = Answers.new(package_1.interpretation('hello3'))
        expected.add_answer(package_2.interpretation('hello2'))
        expected.add_answer(package_3.interpretation('hello4'))
        expected.add_answer(package_4.interpretation('hello5'))

        assert_json expected.to_h, actual

      end

      def test_interpret_parallel_query
        # ici on laisse le serveur charger ses packages au demarrage car le package d'init est trop gros pour un update
        interpretations = []

        # Prepare data
        package = Package.new("package_test_interpret_parallel_query")
        1000.times do |j|
          interpretation = Interpretation.new("interpretation_#{j}").new_textual(["sentence_#{j}"])
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

          assert_equal "#{package.id}", actual['interpretations'].first['package']
          assert_equal "#{interpretation.id}", actual['interpretations'].first['id']
          assert_equal "#{interpretation.slug}", actual['interpretations'].first['slug']
          assert_equal 1.0, actual['interpretations'].first['score'], "Invalid score : #{JSON.pretty_generate(actual)}"

        end
      end

      def test_load_huge_package
        Nls.remove_all_packages

        # Prepare data
        package = Package.new("package_test_interpret_parallel_query")
        1000.times do |j|
          interpretation = Interpretation.new("interpretation_#{j}").new_textual(["sentence_#{j}"])
          package << interpretation
        end

        Nls.package_update(package)

      end

      def test_empty_sentence
        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

        package = available_packages['datetime1']
        expected_error = "NlsCheckRequestString : empty text in request"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret_package(package, "")
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_too_long_sentence
        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

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
          interpretations:
          [
            {
              package: "viky.ai:datetime",
              id: "0d981484-9313-11e7-abc4-cec278b6b50b",
              slug: "special_char",
              score: 1.0
            }
          ]
        }

        assert_json expected, actual
      end

      def test_no_sentence_match
        Nls.remove_all_packages

        several_packages_several_intents.each do |p|
          Nls.package_update(p)
        end

        package = available_packages['datetime1']
        actual = Nls.interpret_package(package, "azerty")

        expected =
        {
          interpretations: []
        }

        assert_json expected, actual

      end

    end

  end

end
