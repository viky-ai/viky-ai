# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < NlsTestCommon

      def test_interpret_simple
        Nls.remove_all_packages

        several_packages_several_intents

        package = available_packages['datetime2']
        Nls.package_update(package)


        sentence = 'Hello Nicolas'
        expected = { interpretation: 'hello1' }

        check_interpret(sentence, expected)

      end

      def test_interpret_several_package_same_sentence

        several_packages_several_intents
        sentence = "Hello Sebastien"

        Nls.remove_all_packages
        package_1 = available_packages['datetime1']
        Nls.package_update package_1
        expected = { interpretation: 'hello3' }
        check_interpret(sentence, expected)

        Nls.remove_all_packages
        package_2 = available_packages['datetime3']
        Nls.package_update package_2
        expected = { interpretation: 'hello2' }
        check_interpret(sentence, expected)

        Nls.remove_all_packages
        package_3 = available_packages['samesentence1']
        Nls.package_update package_3
        expected = { interpretation: 'hello4' }
        check_interpret(sentence, expected)

        Nls.remove_all_packages
        package_4 = available_packages['samesentence2']
        Nls.package_update package_4
        expected = { interpretation: 'hello5' }
        check_interpret(sentence, expected)

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

          sentence = interpretation.expressions.first.expression
          expected = { interpretation: interpretation.slug }
          check_interpret(sentence, expected)

        end
      end

      def test_empty_sentence
        Nls.remove_all_packages

        several_packages_several_intents

        package = available_packages['datetime1']
        Nls.package_update package
        expected_error = "NlpInterpretRequestBuildSentence: sentence is empty"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret_package(package, "")
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_too_long_sentence
        Nls.remove_all_packages

        several_packages_several_intents

        package = available_packages['datetime1']
        Nls.package_update(package)

        sentence = "abcdefghi"
        (64*1024/10).times do
          sentence << "abcdefghij"
        end

        expected_error = "NlpInterpretRequestBuildSentence: too long text in sentence"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.interpret_package(package, sentence)
        end
        assert_response_has_error expected_error, exception, "Post"

      end

      def test_special_char

        cp_import_fixture("package_with_special_char.json")

        Nls.restart

        check_interpret("@#!|\"\\\n_¢ß¥£™©®ª×÷±²³¼½¾µ¿¶·¸",                                   interpretation: 'special_char_1')
        check_interpret("º°¯§…¤¦≠¬ˆ¨‰自爾秦書わたしワタシ🎾",                                     interpretation: 'special_char_2')
        check_interpret("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØŒŠþÙÚÛÜÝŸàáâãäåæçèéêëìíîïðñòóôõöøœšÞùúûüýÿ", interpretation: 'special_char_3')
      end

      def test_no_sentence_match
        Nls.remove_all_packages

        several_packages_several_intents

        package = available_packages['datetime1']
        Nls.package_update(package)

        expected =
        {
          interpretations: []
        }

        check_interpret("azerty", expected)

      end

    end

  end

end
