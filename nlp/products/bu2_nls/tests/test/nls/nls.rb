require 'open3'
require 'term/ansicolor'

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

            Thread.current.name = "nls start : #{command}"
            begin
              Nls.exec(command, { log: log })
            rescue => e
              puts "Nls start crashed : #{e.inspect}"
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
      response  = RestClient.post(url_interpret, body.to_json, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.delete(url, params = {})
      response = RestClient.delete(url, params: params)
      JSON.parse(response.body)
    end

    def self.package(body, params = {})
      package_url = "#{base_url}/packages/" + body["id"]
      response = RestClient.post(package_url, body.to_json, content_type: :json, params: params)
      JSON.parse(response.body)
    end

    def self.json_interpret_body(package = "voqal.ai:datetime1", sentence = "Hello Jean Marie")
      {
        "packages" => ["#{package}"],
        "sentence" => "#{sentence}",
        "Accept-Language" => "fr-FR"
      }
    end

    def self.expected_interpret_result(package = "voqal.ai:datetime1", id = "0d981484-9313-11e7-abc4-cec278b6b50b1", slug = "hello1")
      {
        "interpretations"=>
        [
        {
        "package" => "#{package}",
        "id" => "#{id}",
        "slug" => "#{slug}",
        "score" => 1.0
        }
        ]
      }
    end

    def self.expected_update_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully updated"
      }
    end

    def self.expected_delete_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully deleted"
      }
    end

    def self.expected_added_package(package_name)
      {
        "status" => "Package '#{package_name}' successfully updated"
      }
    end

    def self.pwd
      pwd_local = ENV['NLS_INSTALL_PATH']
      pwd_local = "#{ENV['OG_REPO_PATH']}/ship/debug" if pwd_local.nil?
      pwd_local
    end

    def self.createUUID
      UUIDTools::UUID.timestamp_create.to_s
    end

    def self.package_to_add(slug = "titi", interpretation = "toto", expression = "Hello Brice")
      package_to_update(createUUID, slug, interpretation, expression)
    end

    def self.package_to_update(uuid, slug = "titi", interpretation = "toto", expression="Hello Brice")
      json_expression = create_expression(expression)
      json_interpretation = create_interpretation(interpretation)
      json_interpretation["expressions"] << json_expression
      json_package = create_package(uuid,slug)
      json_package["interpretations"] << json_interpretation
      json_package
    end

    def self.create_package(uuid, slug)
      {
        "id" => "#{uuid}",
        "slug" => "#{slug}",
        "interpretations"  => []
      }
    end

    def self.create_new_package(slug)
      create_package(createUUID, slug)
    end

    def self.create_interpretation(slug = "titi")
      {
        "id" => createUUID,
        "slug"  => "#{slug}",
        "expressions"  =>  []
      }
    end

    def self.create_expression(expression)
      {
        "expression" => "#{expression}",
        "locale"  => "fr-FR"
      }
    end

    def self.several_packages_several_intents
      json_container = []
      json_container << create_new_package("datetime1")
      json_container[0]["interpretations"] << create_interpretation("hello1")
      json_container[0]["interpretations"][0]["expressions"] << create_expression("Hello Brice")
      json_container[0]["interpretations"][0]["expressions"] << create_expression("Hello Jean Marie")
      json_container[0]["interpretations"] << create_interpretation("hello2")
      json_container[0]["interpretations"][1]["expressions"] << create_expression("Hello Nicolas")
      json_container[0]["interpretations"][1]["expressions"] << create_expression("Hello Olivier")
      json_container[0]["interpretations"] << create_interpretation("hello3")
      json_container[0]["interpretations"][2]["expressions"] << create_expression("Hello Sebastien")
      json_container[0]["interpretations"][2]["expressions"] << create_expression("Hello Enrico")
      json_container[0]["interpretations"][2]["expressions"] << create_expression("Hello Mouadh")
      json_container << create_new_package("datetime2")
      json_container[1]["interpretations"] << create_interpretation("hello1")
      json_container[1]["interpretations"][0]["expressions"] << create_expression("Hello Nicolas")
      json_container[1]["interpretations"][0]["expressions"] << create_expression("Hello Olivier")
      json_container << create_new_package("datetime3")
      json_container[2]["interpretations"] << create_interpretation("hello2")
      json_container[2]["interpretations"][0]["expressions"] << create_expression("Hello Sebastien")
      json_container[2]["interpretations"][0]["expressions"] << create_expression("Hello Enrico")
      json_container[2]["interpretations"][0]["expressions"] << create_expression("Hello Mouadh")
      json_container
    end

    def self.several_packages_same_sentence(packagenumber, itentsnumber = 0, sentensenumber = 0)

      if(itentsnumber == 0)
        itentsnumber = packagenumber
      end
      if(sentensenumber == 0)
        sentensenumber = itentsnumber
      end

      json_content_packages = []
      for i in 1..packagenumber
        json_content_packages << create_new_package("package_#{i}")
        for j in 1..itentsnumber
          json_content_packages[i-1]["interpretations"] << create_interpretation("interpretation_#{i}_#{j}")
          json_content_packages[i-1]["interpretations"][j-1]["expressions"] << create_expression("expression_#{j}")
        end
      end

      json_content_packages

    end

    def self.createHugePackagesFile(packagenumber, itentsnumber = 0, sentensenumber = 0)

      if(itentsnumber == 0)
        itentsnumber = packagenumber
      end
      if(sentensenumber == 0)
        sentensenumber = itentsnumber
      end

      json_content_packages = []
      for i in 1..packagenumber
        json_content_packages << create_new_package("package_slug#{i}")
        for j in 1..itentsnumber
          json_content_packages[i-1]["interpretations"] << create_interpretation("interpretation_#{i}_#{j}")
          for k in 1..sentensenumber
            json_content_packages[i-1]["interpretations"][j-1]["expressions"] << create_expression("expression_#{i}_#{j}_#{k}")
          end
        end
      end

      json_content_packages

    end

    def self.package_without_error
      json_packages = []
      json_packages << create_new_package("datetime")
      json_packages[0]["interpretations"] << create_interpretation("hello")
      json_packages[0]["interpretations"][0]["expressions"] << create_expression("Hello Brice")
      json_packages
    end

    private

    def self.exec(cmd, opts = {})

      # defaulr values
      take_care_of_return  = true
      log                  = false

      take_care_of_return = opts[:take_care_of_return] if opts.has_key?(:take_care_of_return)
      log                 = opts[:log]                 if opts.has_key?(:log)

      if File.directory? pwd

        Open3.popen3(cmd, :chdir => pwd, :err=>:out) do |stdin, stdout, stderr, wait_thr|

          pid = wait_thr[:pid]

          #c = Term::ANSIColor

          stdout_str = "" ;
          stderr_str = "" ;

          stdout.each do |line|
            if log
              print "#{line}"
            end
            stdout_str << line ;
          end

          stderr.each do |line|
            if log
              print "#{line}"
            end
            stderr_str << line ;
          end

          # Process::Status object returned.
          exit_status = wait_thr.value

          # you can add a callback block if needed
          if block_given?
            yield exit_status
          end

          if take_care_of_return && !exit_status.success?
            raise "Command \"#{cmd}\" failed !!! (cwd: #{pwd})\r#{stdout_str}\r#{stderr_str}"
          end

          return pid

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
