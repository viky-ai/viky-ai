require 'test_helper'

module Nls

  class TestNlsHelloGet < Common

    def test_hello_world

      actual = Nls.query_get({})

      expected = {
        "hello" => "world"
      }

      assert_equal expected, actual
    end

    def test_hello

      data = {
        name: "toto"
      }

      actual = Nls.query_get(data)

      expected = {
        "hello" => "toto"
      }

      assert_equal expected, actual
    end

  end

  class TestNlsHelloPostByParameters < Common

    def test_hello_world

      actual = Nls.query_post_by_parameters({})

      expected = {
        "hello" => "world"
      }

      assert_equal expected, actual
    end

    def test_hello

      data = {
        name: "toto"
      }

      actual = Nls.query_post_by_parameters(data)

      expected = {
        "hello" => "toto"
      }

      assert_equal expected, actual
    end

  end

  class TestNlsHelloPostByBody < Common

    def test_hello_world

      actual = Nls.query_post_by_body({})

      expected = {
        "hello" => "world"
      }

      assert_equal expected, actual
    end

    def test_hello

      data = {
        name: "toto"
      }

      actual = Nls.query_post_by_body(data)

      expected = {
        "hello" => "toto"
      }

      assert_equal expected, actual
    end

    def test_json_bad_formatted

      begin

        exception = assert_raises RestClient::ExceptionWithResponse do
          RestClient.post(Nls.url, "{\"toto\"}", content_type: :json)
        end

        actual = JSON.parse(exception.response.body)
        expected_error = "OgListeningProcessSearchRequest : Your json contains error in"

        assert actual["errors"].first.include? expected_error
      end

    end


  end

end
