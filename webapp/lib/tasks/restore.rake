require 'pg'
require 'rainbow'

namespace :db do

  desc 'Restore a database dump'
  task :restore, [:file] => [:environment] do |t, args|
    check_arguments(args)
    params = extract_params(args)
    puts Rainbow("Restoring database dump : #{params[:file]}").green
    check_duplicate_db(params)
    create_database(params)
    import_dump(params)
    migrate_data
    puts Rainbow("Database restored at : #{params[:database]}").green
  end


  private

    def check_arguments(args)
      abort Rainbow('Need dump file path').yellow unless args.file.present?
    end

    def check_duplicate_db(params)
      is_db_already_present = false
      conn = PG.connect(host: params[:host],
                        port: params[:port],
                        dbname: 'postgres',
                        user: params[:username],
                        password: params[:password]
      )
      conn.exec("SELECT 1 as is_present FROM pg_database WHERE datname='#{params[:database]}'") do |result|
        is_db_already_present = result.field_values('is_present')[0] == '1'
      end
      if is_db_already_present
        abort Rainbow("Already a database with this name. To drop it use : dropdb -h #{params[:host]} -U #{params[:username]} #{params[:database]}").yellow
      end
    end

    def extract_params(args)
      config = Rails.configuration.database_configuration
      {
        file: args.file,
        host: config[Rails.env]['host'],
        username: config[Rails.env]['username'],
        password: config[Rails.env]['password'],
        port: config[Rails.env]['port'],
        database: 'voqalapp_' + args.file.split('_')[1].tr('-', '_').downcase
      }
    end

    def create_database(params)
      conn = PG.connect(host: params[:host],
                        port: params[:port],
                        dbname: 'postgres',
                        user: params[:username],
                        password: params[:password]
      )
      conn.exec("CREATE DATABASE #{params[:database]}")
    end

    def import_dump(params)
      if params[:file].end_with?('.gz')
        `gunzip -c #{params[:file]} | sed 's/OWNER TO superman/OWNER TO #{params[:username]}/ig' | psql -h #{params[:host]} -p #{params[:port]} -U #{params[:username]} #{params[:database]}`
      else
        `psql -h #{params[:host]} -p #{params[:port]} -U #{params[:username]} #{params[:database]} < sed 's/OWNER TO superman/OWNER TO #{params[:username]}/ig' #{params[:file]}`
      end
    end

  def migrate_data
    Rake::Task["db:migrate"].invoke
  end
end
