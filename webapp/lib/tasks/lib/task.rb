require 'rainbow'
require 'open3'
require 'fileutils'

module Task
  require_relative 'task/print.rb'
  require_relative 'task/cmd.rb'
  require_relative 'task/backup.rb'
  require_relative 'task/backup/create.rb'
  require_relative 'task/backup/restore_postgres.rb'
  require_relative 'task/backup/restore_elastic.rb'
  require_relative 'task/backup/restore_uploads.rb'
end
