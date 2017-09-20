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
    actual = nls_query(data)

    assert_false true
    rescue RestClient::ExceptionWithResponse => e
    actual = JSON.parse(e.response.body)
    expected = {
      "error" => ["lt 0: NlsCancelCleanupOnTimeout : Timeout on LT = 0"]
    }

    assert_equal expected, actual
  end

  def test_infinite_loop
    data = {
      wait: "infinite"
    }
    actual = nls_query(data)

    assert_false true
    rescue RestClient::ExceptionWithResponse => e
    actual = JSON.parse(e.response.body)
    expected = {
      "error" => ["lt 0: NlsCancelCleanupOnTimeout : Timeout on LT = 0"]
    }

    assert_equal expected, actual
  end


end
