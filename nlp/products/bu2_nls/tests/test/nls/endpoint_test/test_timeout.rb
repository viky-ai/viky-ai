require 'test_helper'

module Nls

  module EndpointTest

    class TestTimeout < NlsTestCommon

      def test_timeout_with_wait

        skip_timeout_with_ASAN

        data = {
          timeout: 100,
          wait: 150
        }

        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_get(Nls.url_test, data)
        end
        assert_response_has_error expected_error, exception, "Get"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(Nls.url_test, data)
        end
        assert_response_has_error expected_error, exception, "Post by body"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(Nls.url_test, {}, data)
        end
        assert_response_has_error expected_error, exception, "Post by param"

      end

      def test_timeout_with_infinite_loop

        skip_timeout_with_ASAN

        data = {
          timeout: 100,
          wait: "infinite"
        }

        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_get(Nls.url_test, data)
        end
        assert_response_has_error expected_error, exception, "Get"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(Nls.url_test, data)
        end
        assert_response_has_error expected_error, exception, "Post by body"

        exception = assert_raises RestClient::ExceptionWithResponse do
          Nls.query_post(Nls.url_test, {}, data)
        end
        assert_response_has_error expected_error, exception, "Post by param"

      end

      def test_stop_during_request_with_timeout

        skip_timeout_with_ASAN

        thr1 = Thread.new {
          data = {
            timeout: 200,
            wait: "infinite"
          }

          expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

          exception = assert_raises RestClient::ExceptionWithResponse do
            Nls.query_get(Nls.url_test, data)
          end
          assert_response_has_error expected_error, exception
        }

        thr2 = Thread.new {
          sleep(0.1)
          Nls.stop
        }

        thr1.join
        thr2.join

      end

      def test_stop_during_request_without_timeout

        skip_timeout_with_ASAN

        thr1 = Thread.new {

          data = {
            wait: 200
          }

          actual = Nls.query_get(Nls.url_test, data)

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
