require 'test_helper'

class TestNlsParallel < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  def after_run
    Nls.stop
  end

  def test_parallel

    tests_number = *(0..20)
    Parallel.map(tests_number, in_threads: 3) do |test_number|

      data = {
        wait: 50
      }

      actual = nls_query(data)

      expected = {
        "Answer_wait" => 50
      }

      assert_equal expected, actual
    end


  end


end
