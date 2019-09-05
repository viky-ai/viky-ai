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
    full_cmd = "tar czPf backups/#{name}/#{name}_#{datetime_for_filename}_app-uploads-data.tgz public/uploads/store/"
    Task::Cmd::exec(full_cmd, opts)
  end

  def self.elastic(name)
    client = IndexManager.client
    repository_name = 'viky-es-backup_dev'
    client.snapshot.create_repository repository: repository_name, body: {
      type: 'fs',
      settings: { location: "/backup_data/#{name}/es-backup" }
    }
    snapshot_name = "#{name}-statistics-#{datetime_for_filename}"
    client.snapshot.create repository: repository_name, snapshot: snapshot_name, wait_for_completion: true, body: {
      indices: 'stats-*',
      include_global_state: false
    }
    client.snapshot.delete_repository repository: repository_name
  end

end
