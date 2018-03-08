require 'valgrind_common'

module Valgrind

  class TestMassif < ValgrindCommon

    def test_valgrid_massif_visualizer

      filename = File.basename(__FILE__)
      ruby_log_append(filename, "starting massif test")

      # launch the server
      command = []
      command << "G_DEBUG=resident-modules G_SLICE=always-malloc"
      command << "NLP_JS_DUK_GC_PERIOD=1"
      command << "valgrind --tool=massif"
      command << "--time-unit=ms"
      command << "--threshold=0.01"
      command << "--max-snapshots=200"
      command << "--detailed-freq=1"
      command << "--massif-out-file=massif-output.txt"
      command << "./ogm_nls"

      # clean file
      FileUtils.rm_r("#{pwd}/massif-output.txt", :force => true)

      # launch the server
      ruby_log_append(filename, "starting nls server")
      Nls::Nls.start(command: command.join(' '))

      # wait
      ruby_log_append(filename, "sleep 500ms")
      sleep(0.5)

      # launch queries
      interpret_queries(10)

      # stop server
      ruby_log_append(filename, "stopping server")
      Nls::Nls.stop

      # launch visualizer
      ruby_log_append(filename, "Open massif-visualizer #{pwd}/massif-output.txt")
      puts "Open massif-visualizer #{pwd}/massif-output.txt"
      `massif-visualizer #{pwd}/massif-output.txt 1>/dev/null 2>&1 &`

    end

  end

end
