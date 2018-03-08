require 'pg'
require 'rainbow'
require 'terminal-table'

namespace :restore do

  desc 'Restore an environment on the local machine'
  task :all, [:database_dump, :images] => [:environment] do |t, args|
    unless args.database_dump.present?
      Restore::Print::error('Need database dump file path')
      exit 0
    end
    params = extract_params(args)
    restore_database(params)
    with_active_record_connected_to_new_db params do
      migrate_database
      restore_images(params)
      synchronize_NLP
    end

    Restore::Print::success("Restore completed")
    Restore::Print::notice("Do not forget to:")
    Restore::Print::notice("  - Change your environement variable VIKYAPP_DB_NAME_DEV=#{params[:database]}")
    Restore::Print::notice("  - Restart your apps.")
    Restore::Print::notice("Happy coding!")
  end


  desc 'List available environments and associated backups archives'
  task :list, [:environment] => [:environment] do |t, args|
    if args.environment.present?
      env = args.environment.start_with?('viky-') ? args.environment : "viky-#{args.environment}"
      backups = Restore::Backup::list_all(env)
      rows = backups.map { |b| [env, b, Rainbow("./bin/rails restore:auto[#{env},#{b}]").cyan] }
      puts Terminal::Table.new(headings: ['Environment', 'Backup', 'CMD'], rows: rows)
    else
      environments = Restore::Backup::list_environments
      rows = environments.map { |env| [env, Rainbow("./bin/rails restore:list[#{env}]").cyan] }
      puts Terminal::Table.new(headings: ['Environment', 'CMD'], rows: rows)
    end
  end


  desc 'Download and restore an environment on the local machine'
  task :auto, [:environment, :date] => [:environment] do |t, args|
    unless args.environment.present?
      Restore::Print::error("Missing param: environment name")
    end
    unless args.date.present?
      Restore::Print::error("Missing param: backup date")
    end
    unless args.environment.present? && args.date.present?
      exit 0
    end

    Dir.mktmpdir do |dir|
      env = args.environment.start_with?('viky-') ? args.environment : "viky-#{args.environment}"
      Restore::Print::step("Download archives")
      Restore::Backup::download(env, args.date, dir)
      files = Dir.entries(dir)
      db = "#{dir}/#{files.select {|file| file.end_with?('_db-postgresql.dump.gz')}[0]}"
      images = "#{dir}/#{files.select {|file| file.end_with?('_app-uploads-data.tgz')}[0]}"
      Rake::Task["restore:all"].invoke(db, images)
    end
  end


  private

    def time_log
      "[#{DateTime.now.strftime("%FT%T")}]"
    end

    def restore_database(params)
      Restore::Print::step("Restore database")
      check_duplicate_db(params)
      create_database(params)
      import_dump(params)
    end

    def migrate_database
      Restore::Print::step("Migrate database")
      Rake::Task["db:migrate"].invoke
    end


    def check_duplicate_db(params)
      conn = connect_to_db(params, 'postgres')
      conn.exec("SELECT 1 as is_present FROM pg_database WHERE datname='#{params[:database]}'") do |result|
        is_db_already_present = result.field_values('is_present')[0] == '1'
        if is_db_already_present
          Restore::Print::error("Already a database with this name.")
          Restore::Print::error("To drop it use: dropdb -h #{params[:host]} -U #{params[:username]} #{params[:database]}")
          exit 0
        end
      end
    end

    def create_database(params)
      conn = connect_to_db(params, 'postgres')
      conn.exec("CREATE DATABASE #{params[:database]}")
    end

    def import_dump(params)
      cat_cmd = 'cat'
      cat_cmd = 'gunzip -c' if params[:database_dump].end_with?('.gz')

      opts = { env: { 'PGPASSWORD' => params[:password] }, capture_output: true }
      sed_cmd  = "sed -e 's/OWNER TO superman/OWNER TO #{params[:username]}/g'"
      psql_cmd = "psql --no-password -h '#{params[:host]}' -p '#{params[:port]}' -U '#{params[:username]}' -d '#{params[:database]}'"

      Restore::Cmd::exec("#{cat_cmd} #{params[:database_dump]} | #{sed_cmd} | #{psql_cmd}", opts)
    end

    def extract_params(args)
      config = Rails.configuration.database_configuration
      {
        database_dump: args.database_dump,
        host: config[Rails.env]['host'],
        username: config[Rails.env]['username'],
        password: config[Rails.env]['password'],
        port: config[Rails.env]['port'],
        database: 'vikyapp_' + args.database_dump.split("/").last.split('_')[1].tr('-', '_').downcase,
        images: args.images
      }
    end

    def synchronize_NLP
      Restore::Print::step("Synchronize with NLP")
      backup_dir = File.join(Rails.root, 'import', 'development')
      Restore::Print::substep("Stash packages from import/ to #{backup_dir}")
      unless Dir.exist?(backup_dir)
        FileUtils.mkdir backup_dir
        FileUtils.cp(Dir.glob(File.join(Rails.root, 'import', '/*.json')), backup_dir)
      else
        Restore::Print::notice("    [skipping] Stash directory is already present")
      end
      Restore::Print::substep("Reinit NLP")

      begin
        Rake::Task['packages:reinit'].invoke
      rescue => e
        Restore::Print::notice("    #{e.message}")
        Restore::Print::notice("    [skipping] Nlp packages:reinit failed, you uneeds to restart your NLP server")
      end
    end

    def restore_images(params)
      Restore::Print::step("Restore images")
      if params[:images].present?
        backup_dir = File.join(Rails.root, 'public', 'uploads', 'development')

        Restore::Print::substep("Stash packages from public/uploads to #{backup_dir}")
        unless Dir.exist?(backup_dir)
          FileUtils.mkdir backup_dir
          FileUtils.mv(Dir.glob(File.join(Rails.root, 'public', 'uploads', 'cache')), backup_dir)
          FileUtils.mv(Dir.glob(File.join(Rails.root, 'public', 'uploads', 'store')), backup_dir)
        else
          Restore::Print::notice("    [skipping] Stash directory is already present")
        end

        Restore::Print::substep("Extract images")
        Restore::Cmd::exec("tar xf #{params[:images]} -C #{Rails.root}/..", { capture_output: true })
      else
        Restore::Print::step("No images archive found: reset values")
        User.in_batches.update_all(image_data: nil)
        Agent.in_batches.update_all(image_data: nil)
      end
    end

    def connect_to_db(params, database)
      PG.connect(
        host: params[:host],
        port: params[:port],
        dbname: database,
        user: params[:username],
        password: params[:password]
      )
    end

    def with_active_record_connected_to_new_db(params)
      config = ActiveRecord::Base.connection_config
      ActiveRecord::Base.establish_connection(
        adapter: 'postgresql',
        host: params[:host],
        port: params[:port],
        username: params[:username],
        password: params[:password],
        database: params[:database]
      )
      yield
      ActiveRecord::Base.establish_connection(config)
    end

end



module Restore
  module Print
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


  module Backup
    def self.server_ip
      '172.16.16.6'
    end

    def self.list_all(env)
      data = Cmd::exec(
        "rsync --list-only rsync://docker-backup-ro@#{server_ip}:/docker/backup/#{env}/",
        { capture_output: true }
      )
      data.collect { |line| line.split(' ').last }.reject { |line| line == '.' }
    end

    def self.list_environments
      data = Cmd::exec(
        "rsync --list-only rsync://docker-backup-ro@#{server_ip}:/docker/backup/viky-*",
        { capture_output: true }
      )
      data.collect { |line| line.split(' ').last }
    end

    def self.download(env, date, destination)
      Cmd::exec(
        "rsync -za rsync://docker-backup-ro@#{server_ip}:/docker/backup/#{env}/#{date}/#{env}* #{destination}",
        { capture_output: true }
      )
    end
  end


  module Cmd
    def self.exec(cmd, opts = {})
      opts[:env] = {} if opts[:env].nil?

      data = [] if opts[:capture_output]
      Open3.popen2e(opts[:env], cmd, chdir: Rails.root) do |stdin, stdout_and_stderr, wait_thr|
        if opts[:capture_output]
          stdout_and_stderr.each { |line| data << line }
        else
          stdout_and_stderr.each { |line| puts "    ⤷ #{line}" }
        end

        # Process::Status object returned.
        exit_status = wait_thr.value

        unless exit_status.success?
          puts Rainbow("#{time_log} : ").blue + Rainbow("#{cmd}").cyan
          data.each { |line| puts "    ⤷ ---- #{line}" } if opts[:capture_output]
          raise "Command \"#{cmd}\" failed"
        end
      end

      data if opts[:capture_output]
    end
  end
end
