require 'test_helper'

class BrainControllerTest < ActionDispatch::IntegrationTest

  test "json generation" do
    get "/brain.json"
    assert_response :success
    expected = {
      "nodes" => [],
      "links" => []
    }
    assert_equal expected, JSON.parse(response.body)
  end

end
