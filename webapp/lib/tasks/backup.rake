require_relative 'lib/task'

namespace :backup do

  desc "Restore backup (name='dev', skip_stats='false' by default)"
  task :restore, [:name, :skip_stats] => [:environment] do |t, args|
    args.with_defaults(name: "dev")
    args.with_defaults(skip_stats: "false")

    if Task::Backup::RestorePostgres.dump_file_exist?(args.name)
      Task::Print.step("Restore database")
      Task::Backup::RestorePostgres.run(args.name)
    else
      Task::Print.warning("Restore database skipped (no dump file)")
      exit 0
    end

    if args.skip_stats == "false"
      if Task::Backup::RestoreElastic.dump_file_exist?(args.name)
        Task::Print.step("Restore Elastic indexes")
        Task::Backup::RestoreElastic.run(args.name)
      else
        Task::Print.warning("Restore Elastic indexes skipped (no dump files)")
      end
    end

    if Task::Backup::RestoreUploads.dump_file_exist?(args.name)
      Task::Print.step("Restore uploads")
      Task::Backup::RestoreUploads.run(args.name)
    else
      Task::Print.warning("Restore uploads skipped (no dump file)")
    end

    database = Task::Backup::RestorePostgres.database(args.name)
    Task::Print.step("Update .env with VIKYAPP_DB_NAME_DEV=#{database}")
    Task::Cmd.exec("echo \"VIKYAPP_DB_NAME_DEV=#{database}\" >> #{Rails.root}/.env")

    Task::Print.success("Restore completed, do not forget to restart your apps.")
  end

  desc "Create backup (name='dev', skip_stats='false' by default)"
  task :create, [:name, :skip_stats] => [:environment] do |t, args|
    args.with_defaults(name: "dev")
    args.with_defaults(skip_stats: "false")

    Task::Print.step("Clean backup dir")
    Task::Backup.setup_dir(args.name)

    Task::Print.step("Backup database")
    Task::Backup::Create.postgres(args.name)

    Task::Print.step("Backup uploads")
    Task::Backup::Create.uploads(args.name)

    if args.skip_stats == "false"
      Task::Print.step("Backup Elastic indexes")
      Task::Backup::Create.elastic(args.name)
    end
  end

end
