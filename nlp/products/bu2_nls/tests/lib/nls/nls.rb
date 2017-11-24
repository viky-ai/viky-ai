require 'open3'
require 'term/ansicolor'

# Show request body on error
RestClient::ExceptionWithResponse.class_eval do

  # override message method
  def message
    full_message = (@message || default_message)
    full_message + "\n" + http_body if http_body
  end

end

module Nls

  class Nls

    @@nls_background = nil

    def self.start( opts = {} )

      # default values
      command  = './ogm_nls'
      daemon   = true
      log      = false

      command = opts[:command] if opts.has_key?(:command)
      daemon  = opts[:daemon]  if opts.has_key?(:daemon)
      log     = opts[:log]     if opts.has_key?(:log)

      if daemon

        if @@nls_background.nil?

          pidfile = "#{pwd}/ogm_nls.pid"
          FileUtils.rm(pidfile, :force => true)

          starting = true

          @@nls_background = Thread.new {

            Thread.current.name = "Nls : #{command}"
            begin
              Nls.exec(command, { log: log })
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

      else

        Nls.exec(command, { log: log })

      end

    end

    def self.force_stop
      `killall ogm_nls 1>/dev/null 2>&1`
    end

    def self.stop
      pidfile = "#{pwd}/ogm_nls.pid"

      if File.exist?(pidfile)
        `pkill --pidfile #{pidfile}`
      end

      if !@@nls_background.nil? && @@nls_background.alive?
        @@nls_background.join
      end
      @@nls_background = nil

    end

    def self.restart(opt = {})
      Nls.stop
      sleep(0.05)
      Nls.start(opt)
    end

    def self.base_url

      listening_address = ENV['NLS_LISTENNING_ADDRESS']
      if listening_address.nil?
        listening_address = '127.0.0.1:9345'
      else
        listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
      end

      "http://#{listening_address}"
    end

    def self.url_test
      "#{base_url}/test"
    end

    def self.url_interpret
      "#{base_url}/interpret"
    end

    def self.url_dump
      "#{base_url}/dump"
    end

    def self.url_list
      "#{base_url}/list"
    end

    def self.url_packages
      "#{base_url}/packages"
    end

    def self.query_get(url, params = {})
      response  = RestClient.get(url, params: params)
      JSON.parse(response.body)
    end

    def self.query_post(url, body = {}, params = {})
      response  = RestClient.post(url, body.to_json, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.interpret(body, params = {})
      puts "Interpret request :\n #{body.to_json}\n" if verbose?

      body_to_write = body
      body["why-not-matching"] = {"expression" => "any"}
      body["show-explanation"] = false

      File.open(File.join(pwd, "last_interpret_request.json"),"w") do |f|
        f.write(JSON.pretty_generate(body))
      end

      response  = RestClient.post(url_interpret, body.to_json, content_type: :json, params: params)

      puts "Interpret response :\n #{response.body}\n" if verbose?
      JSON.parse(response.body)
    end

    def self.interpret_package(package, sentence, opts = {})
      body = {
        "packages" => ["#{package.id}"],
        "sentence" => "#{sentence}",
        "Accept-Language" => "fr-FR"
      }
      interpret(body)
    end

    def self.delete(url, params = {})
      response = RestClient.delete(url, params: params)
      JSON.parse(response.body)
    end

    def self.package_delete(package)
      package_url = "#{base_url}/packages/#{package.id}"
      delete(package_url)
    end

    def self.package_update(package, params = {})
      package_id = nil
      package_body = nil
      if package.kind_of? Package
        package.to_file(import_dir)
        package_id = "#{package.id}"
        package_body = JSON.pretty_generate(package.to_h)
      elsif package.kind_of? Hash
        package_id = package['id']
        slug = package['slug']
        if slug.nil?
          filename = "package_#{package_id}.json"
        else
          filename = "package_#{slug}_#{package_id}.json"
        end

        package_body = JSON.pretty_generate(package)

        File.open(File.join(File.expand_path(import_dir), filename), "w") do |f|
          f.write(package_body)
        end
      else
        raise "Unsupported package format #{package.inspect}"
      end

      package_url = "#{base_url}/packages/#{package_id}"
      response = RestClient.post(package_url, package_body, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.dump
      response = RestClient.get(url_dump, params: {})
      JSON.parse(response.body)
    end

    def self.import_dir
      File.join(File.expand_path(pwd), "import")
    end

    def self.pwd
      pwd_local = ENV['NLS_INSTALL_PATH']
      pwd_local = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd_local.nil?
      pwd_local
    end

    def self.remove_all_packages
      list_result = query_get(url_list)
      list_result.each do |package_id|
        package_url = "#{base_url}/packages/#{package_id}"
        Nls.delete(package_url)
      end
      Dir[File.join(import_dir, "*.json")].each do |file|
        FileUtils.rm(file, :force => true)
      end
    end

    def self.verbose?
      test_verbose = ENV['TEST_VERBOSE']
      if !test_verbose.nil?
        return true
      else
        return false
      end
    end

    private

    def self.exec(cmd, opts = {})

      # default values
      take_care_of_return  = true
      log                  = false

      take_care_of_return = opts[:take_care_of_return] if opts.has_key?(:take_care_of_return)
      log                 = opts[:log]                 if opts.has_key?(:log)

      if File.directory? pwd

        Open3.popen3(cmd, :chdir => pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

          pid = wait_thr[:pid]

          stdstr = "" ;

          stdout.each do |line|
            if log
              print "#{line}"
            end
            stdstr << line ;
          end

          stderr.each do |line|
            if log
              print "#{line}"
            end
            stdstr << line ;
          end

          # Process::Status object returned.
          exit_status = wait_thr.value

          # you can add a callback block if needed
          if block_given?
            yield exit_status
          end

          if take_care_of_return && !exit_status.success?
            raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd}, pid:#{pid})\n\n#{stdstr}\n"
          end

        end

      else
        raise "Command \"#{cmd}\" failed, No such PWD dir : #{pwd}"
      end

    rescue => e

      # if we take care raise else print message
      if take_care_of_return
        raise
      else
        c = Term::ANSIColor
        print c.cyan, pwd, " : ", c.blue, c.bold, "#{cmd}", c.clear, " " , c.red, "FAILED", c.clear, " ", e.message, "\n"
      end


    end

  end

end
