require 'test_helper'

module Nls

  module Timeout

    class TestNlsTimeoutPostByParameters < Common

      def test_timeout

        data = {
          timeout: 150,
          wait: 5000
        }

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post_by_parameters(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end

      def test_infinite_loop
        data = {
          timeout: 150,
          wait: "infinite"
        }

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post_by_parameters(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end

      def test_stop_during_request_with_timeout

        thr1 = Thread.new {
          begin
            data = {
              timeout: 200,
              wait: "infinite"
            }
            exception = assert_raises RestClient::ExceptionWithResponse do
              Nls.query_post_by_parameters(data)
            end

            actual = JSON.parse(exception.response.body)
            expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

            assert actual["errors"].first.include? expected_error
          end
        }

        thr2 = Thread.new {
          sleep(0.1)
          Nls.stop
        }

        thr1.join
        thr2.join

      end

      def test_stop_during_request_without_timeout

        thr1 = Thread.new {

          data = {
            wait: 200
          }

          actual = Nls.query_post_by_parameters(data)

          expected = {
            "wait" => 200
          }

          assert_equal expected, actual

        }

        thr2 = Thread.new {
          sleep(0.1)
          Nls.stop
        }

        thr1.join
        thr2.join
      end


    end

  end

end
