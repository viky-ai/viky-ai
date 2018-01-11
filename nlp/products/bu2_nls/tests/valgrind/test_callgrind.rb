require 'valgrind_common'

module Valgrind

  class TestCallGrind < ValgrindCommon

    def test_valgrind_callgrind
      supression_file = File.join(File.expand_path(__dir__), 'valgrind_supression.supp')

      command = []
      command << "valgrind --tool=callgrind"
      command << "--instr-atstart=no"
      command << "--error-limit=no"
      command << "--dump-line=yes"
      command << "--callgrind-out-file=callgrind.out"
      command << "--collect-jumps=yes"
      command << "./ogm_nls -n"

      # clean file
      FileUtils.rm_r("#{pwd}/callgrind.out*", :force => true)

      # launch the server
      Nls::Nls.start(command: command.join(' '))

      # wait
      sleep(0.5)

      # launch queries
      interpret_queries(1, true)

      `callgrind_control -i on`
      `callgrind_control --zero`

      sleep(0.5)

      # launch queries
      interpret_queries(1, true)

      sleep(0.5)

      `callgrind_control --dump`
      `callgrind_control -i off`

      # stop server
      Nls::Nls.stop

      puts "Check time occupation in file #{pwd}/callgrind.out.1"
      `kcachegrind #{pwd}/callgrind.out.1 1>/dev/null 2>&1 &`

    end

  end

end
