require 'test_helper'

class RackThrottleTest < ActionDispatch::IntegrationTest

  setup do
    IndexManager.reset_indices

    # Enable Rack-Throttle
    Feature.enable_rack_throttle

    # remove all keys current Redis DB
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4" })
    redis.flushdb

    #Disable all limits
    Feature.disable_rack_throttle_limit_second
    Feature.disable_rack_throttle_limit_minute
    Feature.disable_rack_throttle_limit_hour
    Feature.disable_rack_throttle_limit_day

    #stub nlp response
    @intent = intents(:weather_forecast)
    @agent = @intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
      status: '200',
      body: {
        'interpretations' => [
          {
            'package' => @agent.id,
            'id'      => @intent.id,
            'slug'    => 'weather_forecast',
            'score'   => 1.0
          }
        ]
      }
    )
  end

  teardown do
    Feature.disable_rack_throttle
  end

  test 'Should be limited if more than 2 requests are made in one second' do
    Feature.enable_rack_throttle_limit_second
    
    travel_to DateTime.current do
      send_interpret_request
      assert_equal '200', response.code
      
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '403', response.code
    end
  end

  test 'Should be limited if more than 4 requests are made in one minute' do
    Feature.enable_rack_throttle_limit_minute

    travel_to DateTime.parse('2019-09-12T14:07:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code
    end

    travel_to DateTime.parse('2019-09-12T14:07:04') do
      send_interpret_request
      assert_equal '200', response.code
    end
    
    travel_to DateTime.parse('2019-09-12T14:07:06') do
      send_interpret_request
      assert_equal '200', response.code
    end
    
    travel_to DateTime.parse('2019-09-12T14:07:07') do
      send_interpret_request
      assert_equal '403', response.code
    end
  end
  
  test 'Should be limited if more than 6 requests are made in one hour' do
    Feature.enable_rack_throttle_limit_hour
    
    travel_to DateTime.parse('2019-09-12T14:00:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code
    end

    travel_to DateTime.parse('2019-09-12T14:20:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code
    end

    travel_to DateTime.parse('2019-09-12T14:45:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '403', response.code
    end

    travel_to DateTime.parse('2019-09-12T15:00:00') do
      send_interpret_request
      assert_equal '200', response.code
    end
  end
  
  test 'Should be limited if more than 8 requests are made in one day' do
    Feature.enable_rack_throttle_limit_day
    
    travel_to DateTime.parse('2019-09-12T14:45:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code
    end

    travel_to DateTime.parse('2019-09-12T20:00:00') do
      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '200', response.code

      send_interpret_request
      assert_equal '403', response.code
    end

    travel_to DateTime.parse('2019-09-13T01:00:00') do
      send_interpret_request
      assert_equal '200', response.code
    end
  end

  private

  def send_interpret_request
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: @agent.api_token }
  end

end
