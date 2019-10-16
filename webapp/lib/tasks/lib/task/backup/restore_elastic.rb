# frozen_string_literal: true

class Task::Backup::RestoreElastic < Task::Backup
  def self.run(name)
    restore_snapshot(name)
    Task::Print.substep('Reindex statistics')
    Rake::Task['statistics:reindex:all'].invoke
  rescue Faraday::ConnectionFailed, ArgumentError => e
    Task::Print.warning("No statistics where restored : #{e.inspect}")
  end

  def self.dump_file_exist?(name)
    Dir.exist?(File.join(dir, name, 'es-backup'))
  end

  def self.restore_from_s3?(name)
    !Dir.glob('**/.from_s3', base: File.join(dir, name)).empty?
  end

  def self.restore_snapshot(name)
    from_s3 = restore_from_s3?(name)
    Task::Print.substep("Restore statistics snapshot (from: #{name}, s3: #{from_s3})")

    client = InterpretRequestLogClient.long_waiting_client
    if from_s3
      repository_name = "s3_restore_#{name}"
      client.create_s3_restore_repository(repository_name, name)
    else
      unless dump_file_exist?(name)
        Task::Print.warning('Restore Elastic indexes skipped (no dump files)')
        return
      end
      repository_name = "local_repository_#{name}"
      client.create_local_repository(repository_name, name)
    end
    client.clear_indices
    client.restore_most_recent_snapshot(repository_name)
  end
end
