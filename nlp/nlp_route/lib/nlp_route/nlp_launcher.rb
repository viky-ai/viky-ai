# frozen_string_literal: true

require 'redis'
require 'open3'
require 'term/ansicolor'
require 'fileutils'
require 'rest-client'
require 'parallel'

module NlpRoute
  class NlpLauncher
    @@nls_background = nil

    def pidfile()
      pidfile = "#{NlpLauncher.pwd}/ogm_nls.pid"
    end

    # get all package from webapp in json format and store in /nlp/import/ then start nlp
    def start(start_nlp = true)
      if start_nlp
        FileUtils.rm(pidfile, force: true)

        starting = true

        @@nls_background = Thread.new do
          command = './ogm_nls'
          Thread.current.name = "Nls : #{command}"
          begin
            NlpLauncher.exec(command)
          rescue StandardError => e
            puts "Nls crashed : #{e.inspect}"
            starting = false
          end
        end

        # wait for starting
        sleep(0.05) while starting && !File.exist?(pidfile)

        raise 'Stopping' unless File.exist?(pidfile)

      end

      # Send all package to NLP
      init

      # subscribe
      begin
        tries ||= 10
        subscribe
      rescue StandardError => e
        if @subcribe_was_success
          puts "Subscribe ends : #{e.inspect}"
          sleep 3
          retry unless (tries -= 1).zero?
          raise 'Subscribe failed'
        else
          puts "Subscribe failed : #{e.inspect}"
          raise 'Subscribe failed'
        end
      end
    end

    def stop
      `pkill --signal SIGTERM --pidfile #{pidfile}` if File.exist?(pidfile)

      @@nls_background.join if !@@nls_background.nil? && @@nls_background.alive?
      @@nls_background = nil
    end

    def init
      start_time = Time.now
      puts 'Loading all packages ...'

      # Load all packages ids
      wrapper = PackageApiWrapper.new
      packages_ids = wrapper.list_id
      payloads = Parallel.map(packages_ids, in_threads: Parallel.processor_count) do |package_id|
        payloads = []
        begin
          retries ||= 3

          package = wrapper.get_package(package_id)
          wrapper.update_or_create(package_id, package)

          payloads << {
            package_id: package_id,
            status: 'success',
            version: package[:version],
            nlp_updated_at: DateTime.now
          }
        rescue StandardError => e
          if (retries -= 1).zero?
            puts "Init package #{package_id} failed (forever) : #{e.inspect}"
            raise "Init package #{package_id} failed (forever)"
          else
            puts "Init package #{package_id} failed (retrying #{retries})"
            sleep 0.3
            retry
          end
        end
        payloads
      end

      if ENV.fetch('NLS_WAIT_TO_BE_READY') { 'false' } == 'true'
        wrapper.set_ready
      end

      if ENV.fetch('VIKYAPP_RUN_TESTS_ON_INIT') { 'true' } == 'true'
        payloads.flatten.each do |payload|
          wrapper.notify_updated_package(payload[:package_id], payload)
        end
      end

      duration = Time.now - start_time

      nls_memory = `ps -p $(cat #{pidfile}) -o rss | head -n 2 | tail -n +2`.strip.to_i rescue 0

      puts "Loading all (#{packages_ids.size}) packages in #{'%.2f' % duration}s using #{nls_memory / 1024} MB memory."

    end

    def subscribe
      # setup
      redis_opts = {
        url: ENV.fetch('VIKYAPP_REDIS_PACKAGE_NOTIFIER') { 'redis://localhost:6379/3' }
      }
      redis = Redis.new(redis_opts)

      # subscribe
      redis_channels = [:viky_packages_change_notifications]
      redis.subscribe(redis_channels) do |on|
        on.subscribe do |channel, subscriptions|
          @subcribe_was_success = true
          puts "Subscribed to ##{channel} (#{subscriptions} subscriptions) ..."
        end

        on.message do |channel, message|
          puts "#{channel}: #{message}"
          parsed_message = JSON.parse(message)

          # restart message
          if parsed_message['event'] == 'unsubscribe'
            redis.unsubscribe
          elsif parsed_message['event'] == 'reinit'
            redis.unsubscribe
            stop
            start(true)
          else

            # processed async
            Thread.new do
              id = parsed_message['id']
              begin
                retries ||= 3

                wrapper = PackageApiWrapper.new
                if parsed_message['event'] == 'update'
                  package = wrapper.get_package(id)
                  wrapper.update_or_create(id, package)

                  notify_payload = {
                    status: 'success',
                    version: package[:version],
                    nlp_updated_at: DateTime.now
                  }
                  wrapper.notify_updated_package(id, notify_payload)

                elsif parsed_message['event'] == 'delete'
                  wrapper.delete(id)
                end
              rescue StandardError => e
                if (retries -= 1).zero?
                  puts "Message processing on package #{id} failed (skipping) : #{e.inspect}"
                else
                  puts "Message processing on package #{id} failed (retrying #{retries})"
                  sleep 0.3
                  retry
                end

                # notify NOT update
                notify_payload = {
                  status: 'failed',
                  version: package[:version],
                  error: e.inspect
                }
                wrapper.notify_updated_package(id, notify_payload)
              end
            end

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
      Open3.popen3(cmd, chdir: pwd, err: :out) do |_stdin, stdout, _stderr, wait_thr|

        # Process::Status object returned.
        exit_status = wait_thr.value

        unless exit_status.success?
          raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd})\n"
        end
      end
    end
  end
end
