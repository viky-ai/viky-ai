module Task::Cmd

  def self.exec(cmd, opts = {}, chdir = Rails.root)
    opts[:env] = {} if opts[:env].nil?

    data = [] if opts[:capture_output]
    Open3.popen2e(opts[:env], cmd, chdir: chdir) do |stdin, stdout_and_stderr, wait_thr|
      if opts[:capture_output]
        stdout_and_stderr.each { |line| data << line }
      else
        stdout_and_stderr.each { |line| puts "    ⤷ #{line}" }
      end

      # Process::Status object returned.
      exit_status = wait_thr.value

      unless exit_status.success?
        Task::Print.step cmd
        data.each do |line|
          Task::Print.substep "⤷ #{line}" if opts[:capture_output]
        end
        raise "Command \"#{cmd}\" failed"
      end
    end

    data if opts[:capture_output]
  end

end
