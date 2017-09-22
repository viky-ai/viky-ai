require 'test_helper'

class TestNlsParallelGet < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = nls_query_get(data)

        expected = {
          "id" => test_number,
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end


end

class TestNlsParallelPostByParameters < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = nls_query_post_by_parameters(data)

        expected = {
          "id" => test_number,
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end


end

class TestNlsParallelPostByBody < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = nls_query_post_by_body(data)

        expected = {
          "id" => test_number,
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end

#  def test_parallel
#
#    tests_number = (0...20).to_a
#    Parallel.map(tests_number, in_threads: 20) do |test_number|
#
#      data = {
#        id: test_number,
#        wait: 50
#      }
#
##      actual = nls_query_post_by_parameters(data)
#      actual = nls_query_get(data)
#
#      expected = {
#        "id" => test_number,
#        "wait" => 50
#      }
#
#      assert_equal expected, actual
#    end
#
#
#  end

end
