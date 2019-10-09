class Task::Backup::RestoreUploads < Task::Backup

  def self.run(name)
    FileUtils.rm_rf File.join(Rails.root, 'public', 'uploads', 'cache')
    FileUtils.rm_rf File.join(Rails.root, 'public', 'uploads', 'store')

    opts = { capture_output: true }
    cmd = "tar xf #{dump_file_path(name)} -C #{Rails.root}/.."

    Task::Cmd::exec(cmd, opts)
  end

  def self.dump_file_exist?(name)
    !dump_file_path(name).nil?
  end

  private

  def self.dump_file_path(name)
    files = Dir.glob(File.join(dir, name, "*_app-uploads-data.tgz"))
    return files.first if files.size == 1
    nil
  end

end
