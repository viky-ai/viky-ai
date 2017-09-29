require 'test_helper'

module Nls

  class TestNlsNlp < Common
    @@testDirPath = File.join(File.expand_path(__dir__),"tmp_test_nls_nlp")
    @@errorJson   = File.join(@@testDirPath,"package_with_error.json")
    @@goodJson    = File.join(@@testDirPath,"package_without_error.json")
    @@severalpackages  = File.join(@@testDirPath,"several_packages_several_intents.json")

    def resetDir
      if File.file?(@@errorJson)
        FileUtils.remove_file(@@errorJson, force = true)
      end
      if File.file?(@@goodJson)
        FileUtils.remove_file(@@goodJson, force = true)
      end
      if File.file?(@@severalpackages)
        FileUtils.remove_file(@@severalpackages, force = true)
      end
      FileUtils.mkdir_p(@@testDirPath)
    end

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
               "sentences" => [
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


    end

  end

end
