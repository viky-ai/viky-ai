require 'test_helper'

class RackThrottleTest < ActionDispatch::IntegrationTest

  setup do
    IndexManager.reset_indices

    # Enable Rack-Throttle
    Feature.enable_rack_throttle
  end

  test 'Should be limited if more than 2 requests are made in one second' do

    # remove all keys current Redis DB
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4" })
    redis.flushdb
    
    Feature.enable_rack_throttle_limit_second
    Feature.disable_rack_throttle_limit_minute
    Feature.disable_rack_throttle_limit_hour
    Feature.disable_rack_throttle_limit_day

    intent = intents(:weather_forecast)
    agent = intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
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
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '403', response.code
    
  end

  test 'Should be limited if more than 4 requests are made in one minute' do
    
    # remove all keys current Redis DB
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4" })
    redis.flushdb

    # Disable no needed limits
    Feature.disable_rack_throttle_limit_second
    Feature.enable_rack_throttle_limit_minute
    Feature.disable_rack_throttle_limit_hour
    Feature.disable_rack_throttle_limit_day
    
    intent = intents(:weather_forecast)
    agent = intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
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
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '403', response.code
    
  end
  
  test 'Should be limited if more than 6 requests are made in one hour' do
    
    # remove all keys current Redis DB
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4" })
    redis.flushdb

    # Disable no needed limits
    Feature.disable_rack_throttle_limit_second
    Feature.disable_rack_throttle_limit_minute
    Feature.enable_rack_throttle_limit_hour
    Feature.disable_rack_throttle_limit_day
    
    intent = intents(:weather_forecast)
    agent = intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
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
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '403', response.code
    
  end
  
  test 'Should be limited if more than 8 requests are made in one day' do
    
    # remove all keys current Redis DB
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4" })
    redis.flushdb

    # Disable no needed limits
    Feature.disable_rack_throttle_limit_second
    Feature.disable_rack_throttle_limit_minute
    Feature.disable_rack_throttle_limit_hour
    Feature.enable_rack_throttle_limit_day
    
    intent = intents(:weather_forecast)
    agent = intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
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
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '200', response.code
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: agent.api_token }
    assert_equal '403', response.code
    
  end

end
