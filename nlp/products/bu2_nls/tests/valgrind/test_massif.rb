require 'valgrind_common'

module Valgrind

  class TestMassif < ValgrindCommon

    def test_valgrid_massif_visualizer

      # launch the server
      command = []
      command << "G_DEBUG=resident-modules G_SLICE=always-malloc"
      command << "valgrind --tool=massif"
      command << "--time-unit=ms"
      command << "--threshold=0.1"
      command << "--max-snapshots=200"
      command << "--detailed-freq=1"
      command << "--massif-out-file=massif-output.txt"
      command << "./ogm_nls"

      # clean file
      FileUtils.rm_r("#{pwd}/massif-output.txt", :force => true)

      # launch the server
      Nls::Nls.start(command: command.join(' '))

      # wait
      sleep(0.5)

      # launch queries
      interpret_queries(10)

      # stop server
      Nls::Nls.stop

      # launch visualizer
      puts "Open massif-visualizer #{pwd}/massif-output.txt"
      `massif-visualizer #{pwd}/massif-output.txt 1>/dev/null 2>&1 &`

    end

  end

end
