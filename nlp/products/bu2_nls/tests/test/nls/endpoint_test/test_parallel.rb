require 'test_helper'

module Nls

  module EndpointTest

    class TestParallel < NlsTestCommon

      def test_parallel_get

        tests_number = (0...20).to_a
        Parallel.map(tests_number, in_threads: 20) do |test_number|

          data = {
            id: test_number,
            wait: 50
          }

          actual = Nls.query_get(Nls.url_test, data)

          expected = {
            "id" => "#{test_number}",
            "wait" => 50
          }

          assert_equal expected, actual
        end


      end

      def test_parallel_post_body

        tests_number = (0...20).to_a
        Parallel.map(tests_number, in_threads: 20) do |test_number|

          data = {
            id: test_number,
            wait: 50
          }

          actual = Nls.query_post(Nls.url_test, data, data)

          expected = {
            "id" => test_number,
            "wait" => 50
          }

          assert_equal expected, actual
        end


      end

      def test_parallel_post_param

        tests_number = (0...20).to_a
        Parallel.map(tests_number, in_threads: 20) do |test_number|

          data = {
            id: test_number,
            wait: 50
          }

          actual = Nls.query_post(Nls.url_test, {}, data)

          expected = {
            "id" => "#{test_number}",
            "wait" => 50
          }

          assert_equal expected, actual
        end


      end

    end

  end

end
