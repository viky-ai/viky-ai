require 'test_helper'

module Nls

  class TestNlsNlp < Common
    @@testDirPath = File.join(File.expand_path(__dir__),"tmp_test_nls_nlp")

    def setup
      Nls.stop
    end

    def teardown
      #FileUtils.rm_rf(@@testDirPath)
    end

    def test_toto


      pwd = ENV['NLS_INSTALL_PATH']
      pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd.nil?

      testFilePath = File.join(@@testDirPath,"package_with_error.json")
      FileUtils.mkdir_p(@@testDirPath)
      Dir.glob(File.join(@@testDirPath,"*.*")).each { |file| File.delete(file)}
      FileUtils.cp(fixture_path("package_with_error.json"), testFilePath)

      command = "./ogm_nls -i #{@@testDirPath}"

      exception = assert_raises RuntimeError do
        response = Nls.exec(command, log: false )
        print "hey #{response}"
      end

      assert exception.message.include? "Error 1/2: main: NlsReadImportFile: Json contains error in ligne 1 and column 5"

    end
  end

end
