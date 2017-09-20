require 'test_helper'

class TestNlsTimeout < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  def after_run
    Nls.stop
  end

  def test_timeout

    data = {
      wait: 5000
    }

    nls_query(data)

    assert false
  rescue RestClient::ExceptionWithResponse => e
    actual = JSON.parse(e.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end

  def test_infinite_loop
    data = {
      wait: "infinite"
    }

    nls_query(data)

    assert false
  rescue RestClient::ExceptionWithResponse => e
    actual = JSON.parse(e.response.body)
    expected_error = "NlsCancelCleanupOnTimeout : Request timeout after"

    assert actual["errors"].first.include? expected_error
  end


end
