require 'common'

module Valgrind

  class TestMemcheck < ValgrindCommon

    def test_valgrind_memcheck

      command = []
      command << "valgrind --tool=memcheck -v"
      command << "--leak-check=full"
      command << "--error-limit=no"
      command << "--track-origins=yes"
      command << "--read-var-info=yes"
      command << "--show-reachable=yes"
      command << "--keep-stacktraces=alloc-then-free"
      command << "--log-file=valgrind.log"
      command << "./ogm_nls"

      # clean file
      FileUtils.rm_r("#{pwd}/#{pwd}/valgrind.log", :force => true)

      # launch the server
      Nls::Nls.start(command: command.join(' '))

      # launch queries
      interpret_queries

      # stop server
      Nls::Nls.stop

      puts "Check leaks in file #{pwd}/valgrind.log"
      `xdg-open #{pwd}/valgrind.log 1>/dev/null 2>&1 &`

    end

  end

end
