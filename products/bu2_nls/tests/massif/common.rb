require 'open3'
require 'term/ansicolor'

module Massif

  class Common < Minitest::Test

    @@debug_path = "#{ENV['OG_REPO_PATH']}/ship/debug"
    @@import_path = "#{@@debug_path}/import"
    @@test_path = "#{ENV['OG_REPO_PATH']}/products/bu2_nls/tests"
    @@test_file_path = "#{@@test_path}/test/nls/fixtures"
    @@pwd = "#{ENV['OG_REPO_PATH']}/ship/debug"

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

    def setup

    end

    def getUrlInterpret
      listening_address = ENV['NLS_LISTENNING_ADDRESS']
      if listening_address.nil?
        listening_address = '127.0.0.1:9345'
      else
        listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
      end
      "http://#{listening_address}/interpret"
    end

    def cleanoutputfiles

      Dir.foreach(@@debug_path) do |filename|
        if filename.start_with? "massif.out"
          FileUtils.remove_file("#{@@debug_path}/#{filename}")
        end
      end

      Dir.foreach(@@test_path) do |filename|
        if filename.start_with? "massif.out"
          FileUtils.remove_file("#{@@test_path}/#{filename}")
        end
      end

      Dir.foreach(@@import_path) do |filename|
        FileUtils.remove_file("#{@@import_path}/#{filename}") unless filename == ".." or filename == "."
      end

      if File.file?("#{@@debug_path}/Valgrind.log")
        FileUtils.remove_file("#{@@debug_path}/Valgrind.log")
      end

    end

    def self.exec(cmd, opts_param = {})

      default_opts = {
        take_care_of_return: true,
        log: true
      }

      opts = default_opts.merge(opts_param)

      take_care_of_return = opts[:take_care_of_return]


      if File.directory? @@pwd

        Open3.popen3(cmd, :chdir => @@pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

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
            raise "Command \"#{cmd}\" failed !!! (cwd: #{@@pwd})\r#{stdout_str}\r#{stderr_str}"
          end

          return pid

        end

      else
        raise "Command \"#{cmd}\" failed, No such PWD dir : #{@@pwd}"
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
