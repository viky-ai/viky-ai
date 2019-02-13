require 'test_helper'
require 'model_test_helper'

class AgentRegressionChecksControllerTest < ActionDispatch::IntegrationTest

  def setup
    create_agent_regression_check_fixtures
  end
  #
  # Create
  #
  test 'Create agent regression checks access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_agent_regression_checks_url(users(:admin), agents(:weather)),
      params: {
        regression_check: { sentence: 'Quel temps fait-il à Paris ?', expected: { package: agents(:weather).id, id: Intent.where(intentname: 'weather_forecast').first.id, score: '1.0', solution: Interpretation.where(expression: 'What the weather like tomorrow ?').first.solution.to_json }, now: '2019-01-17T09:00:00+01:00', language: 'fr', agent_id: agents(:weather) },
        format: :js
      }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create agent regression checks forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_agent_regression_checks_url(users(:admin), agents(:weather)),
      params: {
        regression_check: { sentence: 'Quel temps fait-il à Paris ?', expected: { package: agents(:weather).id, id: Intent.where(intentname: 'weather_forecast').first.id, score: '1.0', solution: Interpretation.where(expression: 'What the weather like tomorrow ?').first.solution.to_json }, now: '2019-01-17T09:00:00+01:00', language: 'fr', agent_id: agents(:weather) },
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

  test 'Run agent regression checks forbidden' do
    sign_in users(:confirmed)
    post run_user_agent_agent_regression_checks_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Delete
  #
  test 'Delete non existing regression check' do
    sign_in users(:edit_on_agent_weather)
    delete user_agent_agent_regression_check_path(users(:admin), agents(:weather), '12345')
    assert_response :not_found
  end

  test 'Delete a running test' do
    sign_in users(:edit_on_agent_weather)

    @regression_weather_question.state = 'running'
    @regression_weather_question.save

    delete user_agent_agent_regression_check_path(users(:admin), agents(:weather), @regression_weather_question)
    assert_response :unprocessable_entity
  end

  test 'Delete a test - success' do
    sign_in users(:edit_on_agent_weather)
    delete user_agent_agent_regression_check_path(users(:admin), agents(:weather), @regression_weather_forecast), xhr: true
    assert_response :success
  end
end
