require 'test_helper'

class ApplicationControllerTest < ActionDispatch::IntegrationTest

  test "A request without json format is not accepted" do
    u = users(:admin)
    a = u.agents.first

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret"
    assert_equal '406', response.code
    assert_equal "Format not acceptable. JSON only is accepted.", JSON.parse(response.body)["message"]

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.xml"
    assert_equal '406', response.code
    assert_equal "Format not acceptable. JSON only is accepted.", JSON.parse(response.body)["message"]
  end

end
