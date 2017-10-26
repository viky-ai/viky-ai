module Nls

  class NlsTestCommon < TestCommon

    attr_reader :available_packages

    @available_packages = {}
    @packages_dump = []

    def importDir
      File.join(File.expand_path(pwd), "import")
    end

    def resetDir

      @available_packages = {}
      @packages_dump = []

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

    def cp_import_fixture(file)
      FileUtils.cp(fixture_path(file), importDir)
    end

    def several_packages_several_intents

      datetime1 = Package.new('datetime1')
      datetime1_hello1 = Interpretation.new('hello1')
      datetime1_hello1 << Expression.new('Hello Brice')
      datetime1_hello1 << Expression.new('Hello Jean Marie')
      datetime1 << datetime1_hello1

      datetime1_hello2 = Interpretation.new('hello2')
      datetime1_hello2 << Expression.new('Hello Nicolas')
      datetime1_hello2 << Expression.new('Hello Olivier')
      datetime1 << datetime1_hello2

      datetime1_hello3 = Interpretation.new('hello3')
      datetime1_hello3 << Expression.new('Hello Sebastien')
      datetime1_hello3 << Expression.new('Hello Enrico')
      datetime1_hello3 << Expression.new('Hello Mouadh')
      datetime1 << datetime1_hello3
      @available_packages[datetime1.slug] = datetime1
      @packages_dump << datetime1.to_h

      datetime2 = Package.new('datetime2')
      datetime2_hello1 = Interpretation.new('hello1')
      datetime2_hello1 << Expression.new('Hello Nicolas')
      datetime2_hello1 << Expression.new('Hello Olivier')
      datetime2 << datetime2_hello1
      @available_packages[datetime2.slug] = datetime2
      @packages_dump << datetime2.to_h

      datetime3 = Package.new('datetime3')
      datetime3_hello2 = Interpretation.new('hello2')
      datetime3_hello2 << Expression.new('Hello Sebastien')
      datetime3_hello2 << Expression.new('Hello Enrico')
      datetime3_hello2 << Expression.new('Hello Mouadh')
      datetime3 << datetime3_hello2
      @available_packages[datetime3.slug] = datetime3
      @packages_dump << datetime3.to_h

      datetime4 = Package.new('samesentence1')
      datetime4_hello4 = Interpretation.new('hello4')
      datetime4_hello4 << Expression.new('Hello Sebastien')
      datetime4_hello4 << Expression.new('Hello Enrico')
      datetime4_hello4 << Expression.new('Hello Mouadh')
      datetime4 << datetime4_hello4
      @available_packages[datetime4.slug] = datetime4
      @packages_dump << datetime4.to_h

      datetime5 = Package.new('samesentence2')
      datetime5_hello5 = Interpretation.new('hello5')
      datetime5_hello5 << Expression.new('Hello Sebastien')
      datetime5_hello5 << Expression.new('Hello Enrico')
      datetime5_hello5 << Expression.new('Hello Mouadh')
      datetime5 << datetime5_hello5
      @available_packages[datetime5.slug] = datetime5
      @packages_dump << datetime5.to_h

      @available_packages.values.each do |package|
        package.to_file(importDir)
      end

    end

    def full_minimal_package(package_slug, interpretation_slug, expression)
      package = Package.new(package_slug)
      interpretation = Interpretation.new(interpretation_slug)
      interpretation << Expression.new(expression)
      package << interpretation
      package
    end

    def package_to_update(uuid, slug = "titi", interpretation = "toto", expression="Hello Brice")
      json_package = Package.new(slug, id: uuid)
      json_interpretation = Interpretation.new(interpretation)
      json_interpretation << Expression.new(expression)
      json_package << json_interpretation
      json_package
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
