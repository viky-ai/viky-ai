require 'open3'
require 'term/ansicolor'

class Nls

  @@pid = nil

  def self.start
    @@pid = Nls.exec('./ogm_nls -d') if @@pid.nil?
  end

  def self.stop
    Nls.exec("kill -9 #{@@pid}") unless @@pid.nil?
    @@pid = nil
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
          raise "Command \"#{cmd}\" failed !!!"
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
