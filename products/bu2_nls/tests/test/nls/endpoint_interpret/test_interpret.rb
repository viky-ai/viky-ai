require 'test_helper'

module Nls

  module EndpointInterpret

    class TestInterpret < Common

      def test_interpret_simple

        cp_import_fixture("several_packages_several_intents.json")

        Nls.restart

        param =
        {
          "packages" => ["voqal.ai:datetime1"],
          "sentence" => "Hello Jean Marie",
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "intents" =>
          [
          {
          "package" => "voqal.ai:datetime1",
          "id" => "0d981484-9313-11e7-abc4-cec278b6b50b1",
          "score" => 1
          }
          ]
        }

        assert_equal expected, actual

      end

      def test_interpret_several_package_same_sentence

        cp_import_fixture("several_packages_same_sentence.json")

        Nls.restart

        param =
        {
          "packages" => ["package_2","package_3","package_1","package_9"],
          "sentence" => "sentence_8",
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "intents" =>
          [
          {
          "package" => "package_2",
          "id" => "intent_2_8",
          "score" => 1
          },
          {
          "package" => "package_3",
          "id" => "intent_3_8",
          "score" => 1
          },
          {
          "package" => "package_1",
          "id" => "intent_1_8",
          "score" => 1
          },
          {
          "package" => "package_9",
          "id" => "intent_9_8",
          "score" => 1
          }
          ]
        }

        assert_equal expected, actual

      end

      def test_interpret_parallel_query

        cp_import_fixture("several_packages_parallelize.json")

        Nls.restart

        sentences_array = [];

        for package in 1..10
          for intent in 1..10
            for sentence in 1..10
              sentences_array << ["package_#{package}","intent_#{package}_#{intent}","sentence_#{package}_#{intent}_#{sentence}"]
            end
          end
        end

        Parallel.map(sentences_array, in_threads: 20) do |sentence_array|

          param =
          {
            "packages" => [sentence_array[0]],
            "sentence" => sentence_array[2],
            "Accept-Language" => "fr-FR"
          }

          actual = Nls.interpret(param)

          expected =
          {
            "intents" =>
            [
            {
            "package" => sentence_array[0],
            "id" => sentence_array[1],
            "score" => 1
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

        param =
        {
          "packages" => ["voqal.ai:datetime"],
          "sentence" => "@#!|\"\\\n_¢ß¥£™©®ª×÷±²³¼½¾µ¿¶·¸º°¯§…¤¦≠¬ˆ¨‰ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØŒŠþÙÚÛÜÝŸàáâãäåæçèéêëìíîïðñòóôõöøœšÞùúûüýÿ#自爾秦書わたしワタシ🎾",
          "Accept-Language" => "fr-FR"
        }

        actual = Nls.interpret(param)

        expected =
        {
          "intents" =>
          [
          {
          "package" => "voqal.ai:datetime",
          "id" => "0d981484-9313-11e7-abc4-cec278b6b50b",
          "score" => 1
          }
          ]
        }

        assert_equal expected, actual
      end


    end

  end

end
