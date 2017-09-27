module Nls

  class Common < Minitest::Test

    def setup
      Nls.start
    end

    Minitest.after_run do
      Nls.stop
    end

    def fixture_path(file = '')
      File.join(File.expand_path(__dir__), 'fixtures', file)
    end

  end

end
