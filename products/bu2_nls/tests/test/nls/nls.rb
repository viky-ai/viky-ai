require 'open3'
require 'term/ansicolor'

module Nls

  class Nls

    @@pid = nil

    def self.start(compile_this_dir = "import")
      if @@pid.nil?
        Nls.exec("./ogm_nls -d -i \"#{compile_this_dir}\"")
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

    def self.restart
      Nls.stop
      sleep(0.05)
      Nls.start
    end

    def self.base_url

      listening_address = ENV['NLS_LISTENNING_ADDRESS']
      if listening_address.nil?
        listening_address = '127.0.0.1:9345'
      else
        listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
      end

      "http://#{listening_address}"
    end

    def self.url_test
      "#{base_url}/test"
    end

    def self.url_interpret
      "#{base_url}/interpret"
    end

    def self.url_dump
      "#{base_url}/dump"
    end

    def self.query_get(url, params = {})
      response  = RestClient.get(url, params: params)
      JSON.parse(response.body)
    end

    def self.query_post(url, body = {}, params = {})
      response  = RestClient.post(url, body.to_json, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.interpret(body, params = {})
      response  = RestClient.post(url_interpret, body.to_json, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.exec(cmd, opts_param = {})

      default_opts = {
        take_care_of_return: true,
        log: true
      }

      opts = default_opts.merge(opts_param)

      take_care_of_return = opts[:take_care_of_return]

      pwd = ENV['NLS_INSTALL_PATH']
      pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd.nil?

      if File.directory? pwd

        Open3.popen3(cmd, :chdir => pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

          pid = wait_thr[:pid]

          #c = Term::ANSIColor

          stdout_str = "" ;
          stderr_str = "" ;

          stdout.each do |line|
            if opts[:log]
              print "#{line}"
            end
            stdout_str << line ;
          end

          stderr.each do |line|
            if opts[:log]
              print "#{line}"
            end
            stderr_str << line ;
          end

          # Process::Status object returned.
          exit_status = wait_thr.value

          # you can add a callback block if needed
          if block_given?
            yield exit_status
          end

          if take_care_of_return && !exit_status.success?
            raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd})\r#{stdout_str}\r#{stderr_str}"
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
