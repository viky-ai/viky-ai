class Task::Backup::RestorePostgres < Task::Backup

  def self.run(name)
    database = database(name)

    # Check if database already exist
    postgres_connexion.exec("SELECT 1 as is_present FROM pg_database WHERE datname='#{database}'") do |result|
      is_db_already_present = result.field_values('is_present')[0] == '1'
      if is_db_already_present
        Task::Print::error("Already a database with this name.")
        Task::Print::error("To drop it use: dropdb -h #{pg('host')} -U #{pg('username')} #{database}")
        exit 0
      end
    end

    # Create database
    postgres_connexion.exec("CREATE DATABASE #{database}")

    # Restore database
    opts = { env: { 'PGPASSWORD' => pg('password') }, capture_output: true }
    cat_cmd = "gunzip -c #{dump_file_path(name)}"
    sed_cmd  = "sed -e 's/ OWNER TO superman;/ OWNER TO #{pg('username')};/g'"
    psql_cmd = "psql --no-password -h '#{pg('host')}' -p '#{pg('port')}' -U '#{pg('username')}' -d '#{database}'"
    Task::Cmd::exec("#{cat_cmd} | #{sed_cmd} | #{psql_cmd}", opts)
  end

  def self.database(name)
    "vikyapp_"+ dump_file_path(name).split("/").last.split('_')[1].tr('-', '_').downcase
  end

  def self.dump_file_exist?(name)
    !dump_file_path(name).nil?
  end

  private

  def self.dump_file_path(name)
    files = Dir.glob(File.join(dir, name, "*_db-postgresql.dump.gz"))
    return files.first if files.size == 1
    nil
  end

  def self.postgres_connexion
    PG.connect(
      host: pg(:host),
      port: pg(:port),
      dbname: 'postgres',
      user: pg(:username),
      password: pg(:password)
    )
  end

end
