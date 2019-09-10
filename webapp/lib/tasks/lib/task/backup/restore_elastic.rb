class Task::Backup::RestoreElastic < Task::Backup

  def self.run(name)
    Task::Print::substep('Import statistics snapshot')
    import_snapshot(name)
    Task::Print::substep('Reindex statistics')
    Rake::Task["statistics:reindex:all"].invoke

    rescue Faraday::ConnectionFailed, ArgumentError => e
      Task::Print::warning("No statistics where restored : #{e.inspect}")
  end

  def self.dump_file_exist?(name)
    Dir.glob(File.join(dir, name, "es-backup")).size == 1
  end

  def self.import_snapshot(name)
    client = InterpretRequestLogClient.long_waiting_client
    client.clear_indices
    client.create_repository "/backup_data/#{name}/es-backup"
    client.restore_most_recent_snapshot
    client.delete_repository
  end
end
