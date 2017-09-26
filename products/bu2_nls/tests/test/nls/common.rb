module Nls

  class Common < Minitest::Test

    def setup
      Nls.start
    end

    Minitest.after_run do
      Nls.stop
    end

  end

end
