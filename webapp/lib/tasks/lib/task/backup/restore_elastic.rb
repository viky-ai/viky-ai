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
    location = "/backup_data/#{name}/es-backup"
    repository = "viky-es-backup_#{name}"

    client = IndexManager.client
    if client.cat.indices(index: 'stats-interpret_request_log-*').present?
      client.indices.update_aliases body: { actions: [
        { remove: { index: 'stats-interpret_request_log-*', alias: InterpretRequestLog::INDEX_ALIAS_NAME } },
        { remove: { index: 'stats-interpret_request_log-*', alias: InterpretRequestLog::SEARCH_ALIAS_NAME } }
      ] }
    end

    client.snapshot.create_repository repository: repository, body: {
      type: 'fs',
      settings: { location: location }
    }
    snapshot = client.cat.snapshots(
      repository: repository,
      s: 'end_epoch',
      h: 'id'
    ).split("\n").last

    client.snapshot.restore(
      repository: repository,
      snapshot: snapshot,
      wait_for_completion: true,
      body: {
        indices: 'stats-interpret_request_log-*',
        rename_pattern: 'stats-interpret_request_log-(\w+)-([0-9]+)-(.+)',
        rename_replacement: "stats-interpret_request_log-$1-$2-$3-#{SecureRandom.hex(4)}",
        index_settings: {
          'index.number_of_replicas' => 0
        }
      }
    )
    client.snapshot.delete_repository repository: repository
  end

end
