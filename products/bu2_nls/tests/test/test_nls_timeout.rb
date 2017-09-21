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

  def test_stop_during_request

    tests_number = *(0..1)
    Parallel.map(tests_number, in_threads: 2) do |test_number|

      if test_number == 0

        begin
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

      else
        sleep(1)
        Nls.stop
      end

    end

  end


end
