module Nlp

  class NlpTestCommon < TestCommon

    def nlp(param = [])

      cmd = "cd #{pwd} && ./ognlp #{param.join(' ')}"
      # puts "Executing command : #{cmd}"
      response = `#{cmd}`
      JSON.parse response

    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

    def setup
      FileUtils.rm("#{pwd}/output.json", :force => true)
      FileUtils.rm("#{pwd}/input.json", :force => true)
    end

  end

end
