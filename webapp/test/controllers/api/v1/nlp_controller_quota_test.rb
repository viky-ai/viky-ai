require 'test_helper'

class NlsControllerQuotaTest < ActionDispatch::IntegrationTest

  setup do
    IndexManager.reset_indices
    Quota.reset_cache

    intent = intents(:weather_forecast)
    @agent = intent.agent
    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
      status: '200',
      body: {
        'interpretations' => [
          {
            'package' => @agent.id,
            'id'      => intent.id,
            'slug'    => 'weather_forecast',
            'score'   => 1.0
          }
        ]
      }
    )
  end


  test 'Should be limited if more than 2 requests are made in one second' do
    Feature.with_quota_enabled do
      assert_request_is_200("2019-09-13T14:07:00")
      assert_request_is_200("2019-09-13T14:07:00")
      assert_request_is_429("2019-09-13T14:07:00")
      assert_request_is_200("2019-09-13T14:07:01")
    end
  end


  test 'Should be limited if more than 8 requests are made in one day' do
    Feature.with_quota_enabled do
      assert_request_is_200("2019-09-13T14:00:00")
      assert_request_is_200("2019-09-13T14:01:00")
      assert_request_is_200("2019-09-13T15:00:00")
      assert_request_is_200("2019-09-13T16:00:00")
      assert_request_is_200("2019-09-13T17:00:00")
      assert_request_is_200("2019-09-13T18:00:00")
      assert_request_is_200("2019-09-13T19:00:00")
      assert_request_is_200("2019-09-13T20:00:00")
      assert_request_is_429("2019-09-13T21:00:00")
      assert_request_is_200("2019-09-14T20:00:00")
    end
  end


  private

  def assert_request_is_200(at)
    travel_to DateTime.parse(at) do
      send_interpret_request
      assert_equal '200', response.code
    end
  end

  def assert_request_is_429(at)
    travel_to DateTime.parse(at) do
      send_interpret_request
      assert_equal '429', response.code
    end
  end

  def send_interpret_request
    get '/api/v1/agents/admin/weather/interpret.json',
      params: { sentence: 'bonjour', agent_token: @agent.api_token }
  end

end
