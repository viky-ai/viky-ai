require 'test_helper'

class BrainControllerTest < ActionDispatch::IntegrationTest

  test "json generation" do
    get "/brain.json"
    assert_response :success
    rsp = JSON.parse(response.body)
    assert_equal 2, rsp["nodes"].size
    assert_equal ["T-800", "terminator_find"], rsp["nodes"].collect {|node| node['name']}
    assert_equal 1, rsp["links"].size
    assert_not rsp["links"][0]["isBetweenAgents"]
    assert rsp["links"][0]["isBetweenAgentAndIntent"]
    assert_not rsp["links"][0]["isBetweenIntents"]
  end

end