class Task::Backup

  def self.pg(attribute)
    config = Rails.configuration.database_configuration
    config[Rails.env][attribute]
  end

  def self.datetime_for_filename
    Time.now.strftime('%Y-%m-%dT%H-%M-%S').downcase
  end

  def self.dir
    File.join(Rails.root, 'backups')
  end

  def self.setup_dir(prefix)
    FileUtils.rm_rf File.join(dir, prefix)
    FileUtils.mkdir File.join(dir, prefix)
  end

end
