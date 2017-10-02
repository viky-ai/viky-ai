module Nls

  class Common < Minitest::Test
    @@testDirPath = File.join(File.expand_path(__dir__),"tmp_test_nls_nlp")
    @@errorJson   = File.join(@@testDirPath,"package_with_error.json")
    @@goodJson    = File.join(@@testDirPath,"package_without_error.json")
    @@severalpackages  = File.join(@@testDirPath,"several_packages_several_intents.json")
    @@parallelpackages  = File.join(@@testDirPath,"several_packages_parallelize.json")
    @@severalpackagessamesentence  = File.join(@@testDirPath,"several_packages_same_sentence.json")

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
      if File.file?(@@parallelpackages)
        FileUtils.remove_file(@@parallelpackages, force = true)
      end
      if File.file?(@@severalpackagessamesentence)
        FileUtils.remove_file(@@severalpackagessamesentence, force = true)
      end
      FileUtils.mkdir_p(@@testDirPath)
    end

    def setup
      Nls.start
    end

    Minitest.after_run do
      Nls.stop
    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

  end

end
