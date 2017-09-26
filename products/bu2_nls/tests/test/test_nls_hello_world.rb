require 'test_helper'

class TestNlsHelloGet < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_hello_world

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

    actual = nls_query_get(data)

    expected = {
      "hello" => "toto"
    }

    assert_equal expected, actual
  end

end

class TestNlsHelloPostByParameters < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_hello_world

    actual = nls_query_post_by_parameters({})

    expected = {
      "hello" => "world"
    }

    assert_equal expected, actual
  end

  def test_hello

    data = {
      name: "toto"
    }

    actual = nls_query_post_by_parameters(data)

    expected = {
      "hello" => "toto"
    }

    assert_equal expected, actual
  end

end

class TestNlsHelloPostByBody < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  Minitest.after_run do
    Nls.stop
  end

  def test_hello_world

    actual = nls_query_post_by_body({})

    expected = {
      "hello" => "world"
    }

    assert_equal expected, actual
  end

  def test_hello

    data = {
      name: "toto"
    }

    actual = nls_query_post_by_body(data)

    expected = {
      "hello" => "toto"
    }

    assert_equal expected, actual
  end

  def test_json_bad_formatted

    begin

      exception = assert_raises RestClient::ExceptionWithResponse do
        RestClient.post(nls_url, "{\"toto\"}", content_type: :json)
      end

      actual = JSON.parse(exception.response.body)
      expected_error = "OgListeningProcessSearchRequest : Your json contains error in"

      assert actual["errors"].first.include? expected_error
    end

  end


end
