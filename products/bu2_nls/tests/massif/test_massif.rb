require 'test_helper'

module Massif

  class TestMassif < Common

    def test_standard_query

      # some cleaning
      cleanoutputfiles

      # copy the right import file in the right place
      FileUtils.cp("#{@@test_file_path}/several_packages_several_intents.json", @@import_path)

      # launch the server
      @@pid = spawn("valgrind --tool=massif --stacks=yes --time-unit=ms --massif-out-file=massif-output.txt ./ogm_nls -d -i import", :chdir=>@@pwd)
      Process.detach(@@pid)

      sleep(2)

      # launch standard query
      param =
      {
        "packages" => ["voqal.ai:datetime1"],
        "sentence" => "Hello Jean Marie",
        "Accept-Language" => "fr-FR"
      }

      # getting the url for server
      url_interpret = getUrlInterpret

      for i in 0..1000
        RestClient.post(url_interpret, param.to_json, content_type: :json)
      end

      # get the server pid
      serverPid = nil
      pid_string = `ps -aux | grep "ogm_nls -d" | grep -v grep`
      serverPid = pid_string.split(" ")[1]

      # stop server
      Common.exec("kill #{serverPid}") unless serverPid.nil?
      serverPid = nil

      # launch visualizer
      Common.exec "massif-visualizer #{@@pwd}/massif-output.txt &"


      assert true
    end

  end

end
