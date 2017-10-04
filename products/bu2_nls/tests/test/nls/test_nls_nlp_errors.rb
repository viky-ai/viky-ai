require 'test_helper'

module Nls

  class TestNlsNlpError < Common

    def test_nlp_nls_error_parsing
      resetDir

      FileUtils.cp(fixture_path("package_with_error.json"), @@errorJson)

      command = "./ogm_nls -i #{@@testDirPath}"

      exception = assert_raises RuntimeError do
        Nls.stop
        Nls.exec(command, log: false)
      end

      assert exception.message.include? "Error 1/2: main: NlsReadImportFile: Json contains error in ligne 1 and column 5"
      resetDir
    end

    def test_same_intent_in_package
      resetDir

      FileUtils.cp(fixture_path("several_same_intents_in_package.json"), @@sameintentinpackage)

      command = "./ogm_nls -i #{@@testDirPath}"

      exception = assert_raises RuntimeError do
        Nls.stop
        Nls.exec(command, log: false)
      end

      assert exception.message.include? "intent id"
      resetDir
    end

    def test_same_package_in_file
      resetDir

      FileUtils.cp(fixture_path("several_same_package_in_file.json"), @@samepackageinfile)

      command = "./ogm_nls -i #{@@testDirPath}"

      exception = assert_raises RuntimeError do
        Nls.stop
        Nls.exec(command, log: false)
      end

      assert exception.message.include? "package id"
      resetDir
    end

  end

end
