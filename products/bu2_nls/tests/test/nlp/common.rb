module Nlp

  class Common < TestCommon

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

  end

end
