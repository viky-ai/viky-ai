require 'open3'
require 'term/ansicolor'

module Nls

  class Nls

    @@pid = nil

    def self.start
      if @@pid.nil?
        Nls.exec('./ogm_nls -d')
        sleep(0.5)
        pid_string = `ps -aux | grep "./ogm_nls -d" | grep -v grep`
        @@pid = pid_string.split(" ")[1]
      end
    end

    def self.stop
      unless @@pid.nil?
        Nls.exec("kill #{@@pid}") unless @@pid.nil?
        @@pid = nil
      end
    end

    def self.url

      listening_address = ENV['NLS_LISTENNING_ADDRESS']
      if listening_address.nil?
        listening_address = '127.0.0.1:9345'
      else
        listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
      end

      "http://#{listening_address}/test"
    end

    def self.query_get(parameters)
      response  = RestClient.get(url, params: parameters)
      JSON.parse(response.body)
    end

    def self.query_post_by_parameters(parameters)
      response  = RestClient.post(url, {}.to_json, content_type: :json, params: parameters)
      JSON.parse(response.body)
    end

    def self.query_post_by_body(param)
      response  = RestClient.post(url, param.to_json, content_type: :json)
      JSON.parse(response.body)
    end

    def self.exec(cmd, take_care_of_return = {})

      pwd = ENV['NLS_INSTALL_PATH']
      pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd.nil?

      if File.directory? pwd

        Open3.popen3(cmd, :chdir => pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

          pid = wait_thr[:pid]

          c = Term::ANSIColor

          stdout.each { |line| print "    ⤷ #{line}" }
          stderr.each { |line| print c.red, "    ⤷ #{line}", c.clear }

          # Process::Status object returned.
          exit_status = wait_thr.value

          # you can add a callback block if needed
          if block_given?
            yield exit_status
          end

          if take_care_of_return && !exit_status.success?
            raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd})"
          end

          return pid

        end

      else
        raise "Command \"#{cmd}\" failed, No such PWD dir : #{pwd}"
      end

    rescue => e

      # if we take care raise else print message
      if take_care_of_return
        raise
      else
        c = Term::ANSIColor
        print c.cyan, pwd, " : ", c.blue, c.bold, "#{cmd}", c.clear, " " , c.red, "FAILED", c.clear, " ", e.message, "\n"
      end


    end

  end

end
