require 'test_helper'

class TestNlsHello < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  def after_run
    Nls.stop
  end

  def test_hello_world

    actual = nls_query({})

    expected = {
      "Answer" => "Hello world!"
    }

    assert_equal expected, actual
  end

  def test_hello

    data = {
      name: 'toto'
    }

    actual = nls_query(data)

    expected = {
      "Answer_name" => "Hello toto"
    }

    assert_equal expected, actual
  end

  def test_json_bad_formatted

    begin

      RestClient.post(nls_url, "{\"toto\"}", content_type: :json)

      assert_false true
    rescue RestClient::ExceptionWithResponse => e
      actual = JSON.parse(e.response.body)
      expected = {
        "error" => ["lt 0: Error during json reformat (2) : yajl_status_error : An error occured during the parse.\nparse error: object key and value must be separated by a colon (':')\n                                {\"toto\"}\n                     (right here) ------^\n"],
        "error_number" => 1
      }
      assert_equal expected, actual
    end

  end


end
