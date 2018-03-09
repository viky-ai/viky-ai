require 'redis'
require 'open3'
require 'term/ansicolor'
require 'fileutils'
require 'rest-client'
require 'parallel'

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
        while starting && !File.exist?(pidfile)
          sleep(0.05)
        end

        raise "Stopping" if !File.exist?(pidfile)

      end

      # Send all package to NLP
      init()

      # subscribe
      begin
        tries ||= 10
        subscribe()
      rescue => e
        if @subcribe_was_success
          puts "Subscribe ends : #{e.inspect}"
          sleep 3
          retry unless (tries -= 1).zero?
          raise "Subscribe failed"
        else
          puts "Subscribe failed : #{e.inspect}"
          raise "Subscribe failed"
        end
      end

    end

    def stop
      pidfile = "#{NlpLauncher.pwd}/ogm_nls.pid"
      if File.exist?(pidfile)
        `pkill --signal SIGTERM --pidfile #{pidfile}`
      end

      if !@@nls_background.nil? && @@nls_background.alive?
        @@nls_background.join
      end
      @@nls_background = nil

    end

    def init

      puts "Loading all packages ..."

      # Load all packages ids
      wrapper = PackageApiWrapper.new
      packages_ids = wrapper.list_id
      Parallel.map(packages_ids, in_threads: Parallel.processor_count) do |package_id|

        begin

          retries ||= 3

          package = wrapper.get_package(package_id)
          wrapper.update_or_create(package_id, package)

        rescue => e
          unless (retries -= 1).zero?
            puts "Init package #{package_id} failed (retrying #{retries})"
            sleep 0.3
            retry
          else
            puts "Init package #{package_id} failed (forever) : #{e.inspect}"
            raise "Init package #{package_id} failed (forever)"
          end
        end

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

          puts "#{channel}: #{message}"
          parsed_message = JSON.parse(message)

          # restart message
          if parsed_message["event"] == "unsubscribe"
            redis.unsubscribe
          elsif parsed_message["event"] == "reinit"
            redis.unsubscribe
            self.stop
            self.start(true)
          else

            # processed async
            Thread.new {

              id = parsed_message["id"]
              begin

                retries ||= 3

                wrapper = PackageApiWrapper.new
                if parsed_message["event"] == "update"
                  package = wrapper.get_package(id)
                  wrapper.update_or_create(id, package)
                elsif parsed_message["event"] == "delete"
                  wrapper.delete(id)
                end

              rescue => e
                unless (retries -= 1).zero?
                  puts "Message processing on package #{id} failed (retrying #{retries})"
                  sleep 0.3
                  retry
                else
                  puts "Message processing on package #{id} failed (skipping) : #{e.inspect}"
                end
              end
            }

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
      Open3.popen3(cmd, :chdir => pwd, :err => :out) do |stdin, stdout, stderr, wait_thr|

        stdout.each do |line|
          print "#{line}"
        end

        # Process::Status object returned.
        exit_status = wait_thr.value

        if !exit_status.success?
          raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd})\n"
        end

      end

    end

  end

end
