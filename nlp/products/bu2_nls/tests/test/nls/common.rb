module Nls

  class Common < TestCommon

    def importDir
      File.join(File.expand_path(pwd), "import")
    end

    def resetDir
      FileUtils.rm_r(importDir, :force => true)
      FileUtils.mkdir_p(importDir)
    end

    def setup

      resetDir

      Nls.start
    end

    Minitest.after_run do
      Nls.stop
    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

    def json_from_fixture(filename)
      JSON.parse(File.read(fixture_path(filename)))
    end

    def createHugePackagesFile(filename, packagenumber, itentsnumber = 0, sentensenumber = 0)

      if(itentsnumber == 0)
        itentsnumber = packagenumber
      end
      if(sentensenumber == 0)
        sentensenumber = itentsnumber
      end

      json_content_packages = []
      for i in 1..packagenumber
        json_object_package = {"id"=>"package_#{i}", "slug"=>"package_slug#{i}", "interpretations"=>[]}
        for j in 1..itentsnumber
          json_object_intents = {"id"=>"interpretation_#{i}_#{j}", "slug"=>"interpretation_#{i}_#{j}", "expressions"=>[]}
          for k in 1..sentensenumber
            json_object_sentence = {"expression"=>"expression_#{i}_#{j}_#{k}", "locale"=>"fr-FR"}
            json_object_intents["expressions"] << json_object_sentence
          end
          json_object_package["interpretations"] << json_object_intents
        end
        json_content_packages << json_object_package
      end

      filepath = File.join(File.expand_path(importDir), filename)
      fJson = File.open(filepath,"w")
      fJson.write(JSON.pretty_generate(json_content_packages))
      fJson.close

    end

    def cp_import_fixture(file)
      FileUtils.cp(fixture_path(file), importDir)
    end

    def generate_single_package_file(json_package)
      filename = "package" + json_package["id"] + ".json"
      filepath = File.join(File.expand_path(importDir), filename)
      fJson = File.open(filepath,"w")
      fJson.write(JSON.pretty_generate(json_package))
      fJson.close
    end

    def generate_multiple_package_file(json_packages)
      filename = "packages" + json_packages[0]["id"] + ".json"
      filepath = File.join(File.expand_path(importDir), filename)
      fJson = File.open(filepath,"w")
      fJson.write(JSON.pretty_generate(json_packages))
      fJson.close
    end


    def assert_exception_has_message expected_error, exception, msg = nil

      header = ''
      header = "#{msg} : " if !msg.nil?

      if !expected_error.nil? && !expected_error.empty?
        assert !exception.nil?, "#{header}exception must not be nil : #{exception.inspect}"
        assert !exception.message.nil?, "#{header}exception message must not be nil : #{exception.inspect}"
        assert !exception.message.empty?, "#{header}exception message must not be empty : #{exception.inspect}"

        expected_error_found = false
        expected_error_found = true  if exception.message.include? expected_error

        assert expected_error_found, "#{header}exception message must contains \"#{expected_error}\": \ntoto#{exception.inspect}"

      end

    end

    def assert_json expected, actual, msg = nil

      header = ''
      header = "#{msg} : " if !msg.nil?

      expected_json = {}
      if expected.kind_of?(Array) || expected.kind_of?(Hash)
        expected_json = expected
      elsif expected.kind_of?(String)
        if File.exist?(expected)
          expected_json = JSON.parse(File.read(expected))
        else
          assert false, "#{header}expected file path must exists, expected: #{expected}"
        end
      else
        assert false, "#{header}expected must a file path or a json object, expected: #{expected.inspect}"
      end

      actual_json = {}
      if actual.kind_of?(Array) || actual.kind_of?(Hash)
        actual_json = actual
      elsif actual.kind_of?(String)
        if File.exist?(actual)
          actual_json = JSON.parse(File.read(actual))
        else
          assert false, "#{header}actual file path must exists, actual: #{actual}"
        end
      else
        assert false, "#{header}actual must a file path or a json object, actual: #{actual.inspect}"
      end

      assert_equal expected_json, actual_json, "#{header} json content must be equals"

    end

    def skip_timeout_with_ASAN

      if ENV['OG_ASAN_ENABLE'] == "1"

        # https://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg544042.html
        skip('ASAN false positive with pthread_cancel() - skipped')

      end

    end

  end

end
