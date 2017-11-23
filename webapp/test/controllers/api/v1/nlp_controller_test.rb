require 'test_helper'

class NlsControllerTest < ActionDispatch::IntegrationTest


  test "Interpret route" do
    assert_routing({
      method: 'get',
      path: "api/v1/agents/admin/weather/interpret.json"
    },
    {
      format: 'json',
      controller: 'api/v1/nlp',
      action: 'interpret',
      ownername: 'admin',
      agentname: 'weather'
    })
  end


  test "Agent access not permitted if token not good" do
    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test" }
    assert_equal '401', response.code
    expected = ["Access denied: wrong token."]
    assert_equal expected, JSON.parse(response.body)["errors"]

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test", agent_token: "blablabla" }
    assert_equal '401', response.code
    expected = ["Access denied: wrong token."]
    assert_equal expected, JSON.parse(response.body)["errors"]
  end


  test "Agent token can be specified in the request header and in the request parameters" do
    intent = intents(:weather_greeting)
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        "interpretations" => [
          {
            "package" => agent.id,
            "id"      => intent.id,
            "slug"    => "weather_greeting",
            "score"   => 1.0
          }
        ]
      }
    )

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test", agent_token: agent.api_token }
    assert_equal '200', response.code
    expected = {
      'interpretations' => [
        {
          "id"    => intent.id,
          "slug"  => "admin/weather/weather_greeting",
          "name"  => "weather_greeting",
          "score" => 1.0
        }
      ]
    }
    assert_equal expected, JSON.parse(response.body)

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test" },
      headers: { "Agent-Token" => agent.api_token }
    assert_equal '200', response.code
    assert_equal expected, JSON.parse(response.body)
  end


  test "Agent token in the request parameters overloads agent token in the request header" do
    intent = intents(:weather_greeting)
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        "interpretations" => [
          {
            "package" => agent.id,
            "id"      => intent.id,
            "slug"    => "weather_greeting",
            "score"   => 1.0
          }
        ]
      }
    )

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test" },
      headers: { "Agent-Token" => agent.api_token }
    assert response.code == '200'

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test", agent_token: "wrong_token" },
      headers: { "Agent-Token" => agent.api_token }
    assert response.code == '401'
  end


  test "An empty sentence request is unprocessable" do
    agent = agents(:weather)

    get "/api/v1/agents/admin/weather/interpret.json?agent_token=#{agent.api_token}"
    assert_equal '422', response.code
    assert_equal ["Sentence can't be blank"], JSON.parse(response.body)["errors"]
  end


end
