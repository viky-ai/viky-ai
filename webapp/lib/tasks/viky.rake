namespace :viky do

  desc "Setup database and statistics"
  task :setup => [:environment] do |t, args|
    # Database related
    connected = ::ActiveRecord::Base.connection_pool.with_connection(&:active?) rescue false
    if connected
      Rake::Task["db:migrate"].invoke
    else
      Rake::Task["db:setup"].invoke
    end
    Rake::Task["viky:clear_old_running_entities_imports"].invoke

    # Statistics related
    Rake::Task["statistics:setup"].invoke
    Rake::Task["statistics:reindex:all"].invoke
  end

  desc "Clear running entities imports older than one hour"
  task :clear_old_running_entities_imports => [:environment] do |t, args|
    if ActiveRecord::Base.connection.table_exists? "entities_imports"
      EntitiesImport.running.where("created_at < ?", 1.hours.ago).delete_all
    end
  end

end