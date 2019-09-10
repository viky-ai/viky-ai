class Task::Backup::Create < Task::Backup

  def self.postgres(name)
    opts = { env: { 'PGPASSWORD' => pg('password') }, capture_output: true }
    sed_cmd  = "sed -e 's/ OWNER TO #{pg('username')};/ OWNER TO superman;/g'"
    psql_cmd = "pg_dump --no-password -h '#{pg('host')}' -p '#{pg('port')}' -U '#{pg('username')}' -d '#{pg('database')}'"
    full_cmd = " #{psql_cmd} | #{sed_cmd} | gzip > backups/#{name}/#{name}_#{datetime_for_filename}_db-postgresql.dump.gz"
    Task::Cmd::exec(full_cmd, opts)
  end

  def self.uploads(name)
    opts = { capture_output: true }
    full_cmd = "tar czPf webapp/backups/#{name}/#{name}_#{datetime_for_filename}_app-uploads-data.tgz webapp/public/uploads/store/"
    Task::Cmd::exec(full_cmd, opts, File.expand_path(File.join(Rails.root, "..")))
  end

  def self.elastic(name)
    client = InterpretRequestLogClient.long_waiting_client
    client.create_repository "/backup_data/#{name}/es-backup"
    snapshot_name = "#{name}-statistics-#{datetime_for_filename}"
    client.create_snapshot(snapshot_name)
    client.delete_repository
  end
end
