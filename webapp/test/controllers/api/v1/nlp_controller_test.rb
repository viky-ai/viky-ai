require 'test_helper'

class NlsControllerTest < ActionDispatch::IntegrationTest
  setup do
    @fixtures_path = File.join(
      Rails.root, 'test', 'fixtures', 'controllers', 'api', 'v1', 'nlp'
    )
    @fixtures_layer_path = File.join(
      Rails.root, 'test', 'fixtures', 'models', 'nlp'
    )
  end


  test "Interpret route" do
    user_id  = 'admin'
    agent_id = 'weather'

    # Interpret action
    assert_routing({
      method: 'get',
      path: "api/v1/agents/#{user_id}/#{agent_id}/interpret.json"
    },
    {
      format: 'json',
      controller: 'api/v1/nlp',
      action: 'interpret',
      ownername: user_id,
      agentname: agent_id
    })
  end


  test "Agent access not permitted if token not good" do
    u = users(:admin)
    a = u.agents.first

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test" }
    assert_equal '401', response.code
    expected = ["Access denied: wrong token."]
    assert_equal expected, JSON.parse(response.body)["errors"]

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test", agent_token: "blablabla" }
    assert_equal '401', response.code
    expected = ["Access denied: wrong token."]
    assert_equal expected, JSON.parse(response.body)["errors"]
  end


  test "Agent token can be specified in the request header and in the request parameters" do
    nlp_layer_interpret_200 = JSON.parse(File.read(File.join(@fixtures_layer_path, "nlp_layer_interpret_200.json")))
    nlp_interpret_200 = JSON.parse(File.read(File.join(@fixtures_path, "nlp_interpret_200.json")))

    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: nlp_layer_interpret_200
    )

    u = users(:admin)
    a = u.agents.first

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test", agent_token: a.api_token }
    assert_equal '200', response.code
    assert_equal nlp_interpret_200, JSON.parse(response.body)

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test" },
      headers: { "Agent-Token" => a.api_token }
    assert_equal '200', response.code
    assert_equal nlp_interpret_200, JSON.parse(response.body)
  end


  test "Agent token in the request parameters overloads agent token in the request header" do
    nlp_layer_interpret_200 = JSON.parse(File.read(File.join(@fixtures_layer_path, "nlp_layer_interpret_200.json")))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: nlp_layer_interpret_200
    )

    u = users(:admin)
    a = u.agents.first
    wrong_token = "blablabla"

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test" },
      headers: { "Agent-Token" => a.api_token }
    assert response.code == '200'

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json",
      params: { sentence: "test", agent_token: wrong_token },
      headers: { "Agent-Token" => a.api_token }
    assert response.code == '401'
  end


  test "An empty sentence request is unprocessable" do
    u = users(:admin)
    a = u.agents.first

    get "/api/v1/agents/#{u.username}/#{a.agentname}/interpret.json?agent_token=#{a.api_token}"
    assert_equal '422', response.code
    assert_equal ["Sentence can't be blank"], JSON.parse(response.body)["errors"]
  end

end
