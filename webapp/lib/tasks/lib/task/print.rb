module Task::Print
  def self.step(text)
    puts "#{time_log} " + Rainbow(text).white
  end

  def self.substep(text)
    puts "#{time_log} " + Rainbow("  #{text}").white
  end

  def self.notice(text)
    puts "#{time_log} " + Rainbow(text).yellow
  end

  def self.warning(text)
    puts "#{time_log} " + Rainbow(text).orange
  end

  def self.error(text)
    puts "#{time_log} " + Rainbow(text).red
  end

  def self.success(text)
    puts "#{time_log} " + Rainbow(text).green
  end

  private

  def time_log
    "[#{DateTime.now.strftime("%FT%T")}]"
  end
end
