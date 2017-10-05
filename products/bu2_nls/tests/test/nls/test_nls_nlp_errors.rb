require 'test_helper'

module Nls

  class TestNlsNlpError < Common

    def setup

      # override setup because this test do not need nls to be started

      resetDir

    end

    def test_nlp_nls_error_parsing

      cp_import_fixture("package_with_error.json")

      command = "./ogm_nls"

      exception = assert_raises RuntimeError do
        Nls.stop
        Nls.exec(command, log: false)
      end

      assert_exception_has_message "NlsReadImportFile: Json contains error in ligne 1 and column 5", exception

    end

    def test_package_already_exists

      cp_import_fixture("several_same_package_in_file.json")

      command = "./ogm_nls"

      exception = assert_raises RuntimeError do
        Nls.stop
        Nls.exec(command, log: false)
      end

      assert_exception_has_message "NlpCompilePackage: package with id='package_1' already exists" , exception

    end

  end

end
