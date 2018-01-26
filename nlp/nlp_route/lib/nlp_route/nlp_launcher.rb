require 'redis'
require 'open3'
require 'term/ansicolor'
require 'fileutils'
require 'rest-client'

module NlpRoute

  class NlpLauncher

    @@nls_background = nil

    # get all package from webapp in json format and store in /nlp/import/ then start nlp
    def start(start_nlp = true)

      if start_nlp
        pidfile = "#{NlpLauncher.pwd}/ogm_nls.pid"
        FileUtils.rm(pidfile, :force => true)

        starting = true

        @@nls_background = Thread.new {

          command = "./ogm_nls"
          Thread.current.name = "Nls : #{command}"
          begin
            NlpLauncher.exec(command)
          rescue => e
            puts "Nls crashed : #{e.inspect}"
            starting = false
          end

        }

        # wait for starting
        until starting && File.exist?(pidfile)
          sleep(0.05)
        end
      end

      # Send all package to NLP
      init()

      # subscribe
      begin
        tries ||= 10

        subscribe()

      rescue => e

        if @subcribe_was_success
          puts "Subscribe ends : #{e.inpect}"
          sleep 3
          retry unless (tries -= 1).zero?
        else
          puts "Subscribe failed : #{e.inpect}"
          raise
        end

      end

    end

    def stop
      pidfile = "#{NlpLauncher.pwd}/ogm_nls.pid"

      if File.exist?(pidfile)
        `pkill --pidfile #{pidfile}`
      end

      if !@@nls_background.nil? && @@nls_background.alive?
        @@nls_background.join
      end
      @@nls_background = nil

    end

    def init

      #Load all packages ids
      wrapper = PackageApiWrapper.new
      packages_ids = wrapper.list_id
      packages_ids.each do |package_id|
        package = wrapper.get_package(package_id)
        wrapper.update_or_create(package_id, package)
      end

    end

    def subscribe

      # setup
      redis_opts = {
        url: ENV.fetch("VIKYAPP_REDIS_PACKAGE_NOTIFIER") { 'redis://localhost:6379/3' }
      }
      redis = Redis.new(redis_opts)

      # subscribe
      redis_channels = [ :viky_packages_change_notifications ]
      redis.subscribe(redis_channels) do |on|

        on.subscribe do |channel, subscriptions|
          @subcribe_was_success = true
          puts "Subscribed to ##{channel} (#{subscriptions} subscriptions) ..."
        end

        on.message do |channel, message|
          parsed_message = JSON.parse(message)
          wrapper = PackageApiWrapper.new
          puts "#{channel}: #{parsed_message}"

          id = parsed_message["id"]

          if parsed_message["event"] == "update"
            package = wrapper.get_package(id)
            wrapper.update_or_create(id, package)
          elsif parsed_message["event"] == "delete"
            wrapper.delete(id)
          elsif parsed_message["event"] == "unsubscribe"
            redis.unsubscribe
          end

        end

      end
    end

    def self.pwd
      pwd_local = ENV['NLS_INSTALL_PATH']
      pwd_local = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd_local.nil?
      pwd_local
    end

    def self.exec(cmd)

      Open3.popen3(cmd, :chdir => pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

        stdout.each do |line|
          print "#{line}"
        end

        # Process::Status object returned.
        exit_status = wait_thr.value

        if !exit_status.success?
          raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd}, pid:#{pid})\n\n#{stdstr}\n"
        end

      end


    end

  end

end
