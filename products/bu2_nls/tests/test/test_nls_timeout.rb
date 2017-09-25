require 'test_helper'

class TestNlsTimeoutGet < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_timeout

    data = {
      timeout:100,
      wait: 150
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
      nls_query_get(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_infinite_loop
    data = {
      timeout:100,
      wait: "infinite"
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
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
          timeout:500,
          wait: "infinite"
        }
        exception = assert_raises RestClient::ExceptionWithResponse do
          nls_query_get(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end
    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join

  end

  def test_stop_during_request_without_timeout

    thr1 = Thread.new {

      data = {
        wait: 500
      }

      actual = nls_query_get(data)

      expected = {
        "wait" => 500
      }

      assert_equal expected, actual

    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join
  end


end

class TestNlsTimeoutPostByParameters < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_timeout

    data = {
      timeout:150,
      wait: 5000
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
      nls_query_post_by_parameters(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_infinite_loop
    data = {
      timeout:150,
      wait: "infinite"
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
      nls_query_post_by_parameters(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_stop_during_request_with_timeout

    thr1 = Thread.new {
      begin
        data = {
          timeout:500,
          wait: "infinite"
        }
        exception = assert_raises RestClient::ExceptionWithResponse do
          nls_query_post_by_parameters(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end
    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join

  end

  def test_stop_during_request_without_timeout

    thr1 = Thread.new {

      data = {
        wait: 500
      }

      actual = nls_query_post_by_parameters(data)

      expected = {
        "wait" => 500
      }

      assert_equal expected, actual

    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join
  end


end

class TestNlsTimeoutPostByBody < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_timeout

    data = {
      timeout:150,
      wait: 5000
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
      nls_query_post_by_body(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_infinite_loop
    data = {
      timeout:150,
      wait: "infinite"
    }

    exception = assert_raises RestClient::ExceptionWithResponse do
      nls_query_post_by_body(data)
    end

    actual = JSON.parse(exception.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_stop_during_request_with_timeout

    thr1 = Thread.new {
      begin
        data = {
          timeout:500,
          wait: "infinite"
        }
        exception = assert_raises RestClient::ExceptionWithResponse do
          nls_query_post_by_body(data)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

        assert actual["errors"].first.include? expected_error
      end
    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join

  end

  def test_stop_during_request_without_timeout

    thr1 = Thread.new {

      data = {
        wait: 500
      }

      actual = nls_query_post_by_body(data)

      expected = {
        "wait" => 500
      }

      assert_equal expected, actual

    }

    thr2 = Thread.new {
      sleep(0.2)
      Nls.stop
    }

    thr1.join
    thr2.join
  end


end
