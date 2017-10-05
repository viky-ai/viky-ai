module Nlp

  class Common < Minitest::Test

    @@pwd = ENV['NLS_INSTALL_PATH']
    @@pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if @@pwd.nil?

    def nlp(param = [])

      response = `cd #{@@pwd} && ./ognlp #{param.join(' ')}`
      JSON.parse response

    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

    def setup
      if File.file?("#{@@pwd}output.json")
        File.delete("#{@@pwd}/output.json")
      end
      if File.file?("#{@@pwd}input.json")
        File.delete("#{@@pwd}/input.json")
      end
    end

    def assert_response_has_error expected_error = "", actual_response = [], msg = nil

      header = ''
      header = "#{msg} : " if !msg.nil?

      if !expected_error.nil? && !expected_error.empty?
        assert !actual_response["errors"].nil?, "#{header}actual_response must contains error : \n#{JSON.generate(actual_response)}"
        assert !actual_response["errors"].empty?, "#{header}actual_response errors must not be empty : \n#{JSON.generate(actual_response)}"

        expected_error_found = false
        actual_response["errors"].each do |error|
          if error.include? expected_error
            expected_error_found = true
            break
          end

        end

        assert expected_error_found, "#{header}actual_response must contains error \"#{expected_error}\": \n#{JSON.generate(actual_response)}"

      end

    end

  end

end
