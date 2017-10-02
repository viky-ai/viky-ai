require 'test_helper'

module Nls

  class TestNlsNlp < Common

    def test_nlp_nls_error_parsing
      resetDir

      FileUtils.cp(fixture_path("package_with_error.json"), @@errorJson)

      command = "./ogm_nls -i #{@@testDirPath}"

      exception = assert_raises RuntimeError do
        Nls.stop
        response = Nls.exec(command, log: false)
        print "hey #{response}"
      end

      assert exception.message.include? "Error 1/2: main: NlsReadImportFile: Json contains error in ligne 1 and column 5"
      resetDir
    end

    def test_nlp_nls_compile
      resetDir

      FileUtils.cp(fixture_path("package_without_error.json"), @@goodJson)

      Nls.stop
      sleep(0.1)
      Nls.start(@@testDirPath)

      response  = RestClient.get(Nls.url_dump)
      actual    = JSON.parse(response.body)

      expected =
      [
        {
          "id" => "voqal.ai:datetime",
          "intents" =>
          [
            {
              "id"=>"0d981484-9313-11e7-abc4-cec278b6b50b",
              "sentences" =>
              [
                {
                  "sentence" => "Hello Brice",
                  "locale"=>"fr-FR"
                }
              ]
            }
          ]
        }
      ];

      assert_equal expected, actual
      resetDir
    end

    def test_nlp_nls_query
      resetDir

      FileUtils.cp(fixture_path("several_packages_several_intents.json"), @@severalpackages)

      Nls.stop
      sleep(0.1)
      Nls.start(@@testDirPath)

      param =
      {
        "packages" => ["voqal.ai:datetime1"],
        "sentence" => "Hello Jean Marie",
        "Accept-Language" => "fr-FR"
      }

      response  = RestClient.post(Nls.url_interpret, param.to_json, content_type: :json)
      actual    = JSON.parse(response.body)

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

    def test_nlp_nls_several_package_same_sentence
      resetDir

      FileUtils.cp(fixture_path("several_packages_same_sentence.json"), @@severalpackagessamesentence)

      Nls.stop
      sleep(0.1)
      Nls.start(@@testDirPath)

      param =
      {
        "packages" => ["package_2","package_3","package_1","package_9"],
        "sentence" => "sentence_8",
        "Accept-Language" => "fr-FR"
      }

      response  = RestClient.post(Nls.url_interpret, param.to_json, content_type: :json)
      actual    = JSON.parse(response.body)

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

    def test_nlp_nls_parallel_query
      resetDir

      FileUtils.cp(fixture_path("several_packages_parallelize.json"), @@parallelpackages)

      Nls.stop
      sleep(0.1)
      Nls.start(@@testDirPath)

      sentences_array = [];

      for package in 1..10
        for intent in 1..10
          for sentence in 1..10
            sentences_array << ["package_#{package}","intent_#{package}_#{intent}","sentence_#{package}_#{intent}_#{sentence}"]
          end
        end
      end

      tests_number = sentences_array.size
      Parallel.map(sentences_array, in_threads: 20) do |sentence_array|

        param =
        {
          "packages" => [sentence_array[0]],
          "sentence" => sentence_array[2],
          "Accept-Language" => "fr-FR"
        }

        response  = RestClient.post(Nls.url_interpret, param.to_json, content_type: :json)
        actual    = JSON.parse(response.body)

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

  end

end
