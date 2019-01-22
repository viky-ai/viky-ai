require 'test_helper'

class AgentRegressionChecksControllerTest < ActionDispatch::IntegrationTest
  #
  # Create
  #
  test 'Create agent regression checks access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_agent_regression_checks_url(users(:admin), agents(:weather)),
      params: {
        regression_check: { sentence: 'Quel temps fait-il à Paris ?', expected: { package: agents(:weather).id, id: Intent.where(intentname: 'weather_forecast').first.id, score: '1.0', solution: Interpretation.where(expression: 'What the weather like tomorrow ?').first.solution }, now: '2019-01-17T09:00:00+01:00', language: 'fr', agent_id: agents(:weather) },
        format: :js
      }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create agent regression checks forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_agent_regression_checks_url(users(:admin), agents(:weather)),
      params: {
        regression_check: { sentence: 'Quel temps fait-il à Paris ?', expected: { package: agents(:weather).id, id: Intent.where(intentname: 'weather_forecast').first.id, score: '1.0', solution: Interpretation.where(expression: 'What the weather like tomorrow ?').first.solution }, now: '2019-01-17T09:00:00+01:00', language: 'fr', agent_id: agents(:weather) },
        format: :js
      }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Run test suites
  #
  test 'Run agent regression checks access' do
    sign_in users(:edit_on_agent_weather)
    post run_user_agent_agent_regression_checks_url(users(:admin), agents(:weather))
    assert :success
    assert_nil flash[:alert]
  end

  # test 'Run agent regression checks forbidden' do
  #   sign_in users(:show_on_agent_weather)
  #   post run_user_agent_agent_regression_checks_url(users(:admin), agents(:weather))
  #   assert_redirected_to agents_url
  #   assert_equal 'Unauthorized operation.', flash[:alert]
  # end
end
