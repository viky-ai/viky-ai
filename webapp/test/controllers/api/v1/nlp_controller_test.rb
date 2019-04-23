require 'test_helper'

class NlsControllerTest < ActionDispatch::IntegrationTest

  setup do
    IndexManager.reset_indices
  end

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
    intent = intents(:weather_forecast)
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        "interpretations" => [
          {
            "package" => agent.id,
            "id"      => intent.id,
            "slug"    => "weather_forecast",
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
          "slug"  => "weather_forecast",
          "name"  => "weather_forecast",
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
    intent = intents(:weather_forecast)
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        "interpretations" => [
          {
            "package" => agent.id,
            "id"      => intent.id,
            "slug"    => "weather_forecast",
            "score"   => 1.0
          }
        ]
      }
    )

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test" },
      headers: { "Agent-Token" => agent.api_token }
    assert_equal '200', response.code

    get "/api/v1/agents/admin/weather/interpret.json",
      params: { sentence: "test", agent_token: "wrong_token" },
      headers: { "Agent-Token" => agent.api_token }
    assert_equal '401', response.code
  end


  test "An empty sentence request is unprocessable" do
    agent = agents(:weather)

    get "/api/v1/agents/admin/weather/interpret.json?agent_token=#{agent.api_token}"
    assert_equal '422', response.code
    assert_equal ["Sentence can't be blank"], JSON.parse(response.body)["errors"]
  end


  test 'Provide statistics contexts with the sentence' do
    sentence = "test context #{SecureRandom.uuid}"
    intent = intents(:weather_forecast)
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        'interpretations' => [
          {
            'package' => agent.id,
            'id'      => intent.id,
            'slug'    => 'weather_forecast',
            'score'   => 1.0
          }
        ]
      }
    )

    get '/api/v1/agents/admin/weather/interpret.json',
        params: {
          sentence: sentence,
          agent_token: agent.api_token,
          context: {
            session_id: 'abc',
            bot_version: '1.1-a58b'
          }
        }

    expected = {
      'interpretations' => [
        {
          'id'    => intent.id,
          'slug'  => 'weather_forecast',
          'name'  => 'weather_forecast',
          'score' => 1.0
        }
      ]
    }
    assert_equal expected, JSON.parse(response.body)
    client = IndexManager.client
    result = client.search(
      index: InterpretRequestLog::SEARCH_ALIAS_NAME,
      type: InterpretRequestLog::INDEX_TYPE,
      body: { query: { match: { sentence: sentence } } },
      size: 1
    )
    found = result['hits']['hits'].first['_source'].symbolize_keys
    assert_equal 'abc', found[:context]['session_id']
    assert_equal '1.1-a58b', found[:context]['bot_version']
    assert_equal agent.updated_at, found[:context]['agent_version']
  end


  test 'Set spellchecking' do
    intent = intents(:weather_forecast)
    agent = intent.agent
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: '200',
      body: {
        'interpretations' => [
          {
            'package' => agent.id,
            'id'      => intent.id,
            'slug'    => 'weather_forecast',
            'score'   => 1.0
          }
        ]
      }
    )

    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjoir', agent_token: agent.api_token, spellchecking: 'inactive' }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjoir', agent_token: agent.api_token, spellchecking: 'low' }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjoir', agent_token: agent.api_token, spellchecking: 'medium' }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjoir', agent_token: agent.api_token, spellchecking: 'high' }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjoir', agent_token: agent.api_token, spellchecking: 'foobar' }
    assert_equal '422', response.code
  end


  test 'Log request even when no more NLP server are available' do
    sentence = "test NLP crash #{SecureRandom.uuid}"
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').raises(Errno::ECONNREFUSED, 'Failed to open TCP connection')

    get '/api/v1/agents/admin/weather/interpret.json',
        params: {
          sentence: sentence,
          agent_token: agent.api_token
        }

    client = IndexManager.client
    result = client.search(
      index: InterpretRequestLog::SEARCH_ALIAS_NAME,
      type: InterpretRequestLog::INDEX_TYPE,
      body: { query: { match: { sentence: sentence } } },
      size: 1
    )
    found = result['hits']['hits'].first['_source'].symbolize_keys
    assert_equal 503, found[:status]
    assert_equal ['NLS temporarily unavailable', 'No more NLP server are available', 'Connection refused - Failed to open TCP connection'],
                 found[:body]['errors']
  end

  test 'Log request even when NLP has just crashed' do
    sentence = "test NLP crash #{SecureRandom.uuid}"
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').raises(EOFError, 'end of file reached')

    get '/api/v1/agents/admin/weather/interpret.json',
        params: {
          sentence: sentence,
          agent_token: agent.api_token
        }

    client = IndexManager.client
    result = client.search(
      index: InterpretRequestLog::SEARCH_ALIAS_NAME,
      type: InterpretRequestLog::INDEX_TYPE,
      body: { query: { match: { sentence: sentence } } },
      size: 1
    )
    found = result['hits']['hits'].first['_source'].symbolize_keys
    assert_equal 503, found[:status]
    assert_equal ['NLS temporarily unavailable', 'NLP have just crashed'], found[:body]['errors']
  end

  test 'Log request even when unexpected error' do
    sentence = "test NLP Big big error #{SecureRandom.uuid}"
    agent = agents(:weather)
    Nlp::Interpret.any_instance.stubs('proceed').raises(RuntimeError, 'Big big error')

    assert_raise RuntimeError do
      get '/api/v1/agents/admin/weather/interpret.json',
          params: {
            sentence: sentence,
            agent_token: agent.api_token
          }
    end

    client = IndexManager.client
    result = client.search(
      index: InterpretRequestLog::SEARCH_ALIAS_NAME,
      type: InterpretRequestLog::INDEX_TYPE,
      body: { query: { match: { sentence: sentence } } },
      size: 1
    )
    found = result['hits']['hits'].first['_source'].symbolize_keys
    assert_equal 500, found[:status]
    assert_equal ['NLS temporarily unavailable', '#<RuntimeError: Big big error>'], found[:body]['errors']
  end
end
