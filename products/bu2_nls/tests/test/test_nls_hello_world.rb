require 'test_helper'

class TestNlsHello < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_hello_world

#    actual = nls_query_post_by_parameters({})
    actual = nls_query_get({})

    expected = {
      "hello" => "world"
    }

    assert_equal expected, actual
  end

  def test_hello

    data = {
      name: "toto"
    }

#    actual = nls_query_post_by_parameters(data)
    actual = nls_query_get(data)

    expected = {
      "hello" => "toto"
    }

    assert_equal expected, actual
  end

#  def test_json_bad_formatted
#
#    begin
#
#      exception = assert_raises RestClient::ExceptionWithResponse do
#        RestClient.post(nls_url, "{\"toto\"}", content_type: :json)
#      end
#
#      actual = JSON.parse(exception.response.body)
#
#      expected_error =
#      """
#lt 0: Error during json reformat (2) : yajl_status_error : An error occured during the parse.
#parse error: object key and value must be separated by a colon (':')
#                                {\"toto\"}
#                     (right here) ------^
#"""
#
#      assert_equal expected_error.strip, actual['errors'].first.strip
#    end
#
#  end


end
