require 'test_helper'

module Nls

  class TestNlsParallelGet < Common

    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = Nls.query_get(data)

        expected = {
          "id" => "#{test_number}",
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end


  end

  class TestNlsParallelPostByParameters < Common

    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = Nls.query_post_by_parameters(data)

        expected = {
          "id" => "#{test_number}",
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end


  end

  class TestNlsParallelPostByBody < Common


    def test_parallel

      tests_number = (0...20).to_a
      Parallel.map(tests_number, in_threads: 20) do |test_number|

        data = {
          id: test_number,
          wait: 50
        }

        actual = Nls.query_post_by_body(data)

        expected = {
          "id" => test_number,
          "wait" => 50
        }

        assert_equal expected, actual
      end


    end

  end

end
