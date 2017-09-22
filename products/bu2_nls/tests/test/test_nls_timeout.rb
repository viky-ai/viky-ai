require 'test_helper'

class TestNlsTimeout < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_timeout

    data = {
      wait: 5000
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
#      nls_query_post_by_parameters(data)
      nls_query_get(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_infinite_loop
    data = {
      wait: "infinite"
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
#      nls_query_post_by_parameters(data)
      nls_query_get(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_stop_during_request_with_timeout

    thr1 = Thread.new {
      begin
        data = {
          wait: "infinite"
        }
        exception = assert_raises RestClient::ExceptionWithResponse do
#          nls_query_post_by_parameters(data)
          nls_query_get(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end
    }

    thr2 = Thread.new {
      sleep(1)
      Nls.stop
    }

    thr1.join
    thr2.join

  end

  def test_stop_during_request_without_timeout

    thr1 = Thread.new {

      data = {
        wait: 1500
      }

#      actual = nls_query_post_by_parameters(data)
      actual = nls_query_get(data)

      expected = {
        "hello" => "world"
      }

      assert_equal expected, actual

    }

    thr2 = Thread.new {
      sleep(0.5)
      Nls.stop
    }

    thr1.join
    thr2.join
  end


end
