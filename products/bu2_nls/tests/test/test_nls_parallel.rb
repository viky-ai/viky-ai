require 'test_helper'

require 'parallel'

class TestNlsParallel < Minitest::Test

  # add
  def test_parallel

    url = "http://127.0.0.1:9345"

    tests_number = *(0..20)
    Parallel.map(tests_number, in_threads: 3) do |test_number|
      data = {
        wait: 50
      }

      response  = RestClient.post(url, data.to_json, content_type: :json)
      actual = JSON.parse(response.body)


      expected = {
        "Answer_wait" => 50
      }

      assert_equal expected, actual
    end


  end


end
