require 'application_system_test_case'
require 'model_test_helper'

class AgentRegressionChecksTest < ApplicationSystemTestCase

  def setup
    create_agent_regression_check_fixtures
  end

  test 'Add new regression test' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      first('button').click
      Nlp::Interpret.any_instance.stubs('proceed').returns(
        status: 200,
        body: {
          interpretations: [
            {
              'id' => intents(:weather_forecast).id,
              'slug' => intents(:weather_forecast).slug,
              'package' => intents(:weather_forecast).agent.id,
              'score' => '1.0',
              'solution' => interpretations(:weather_forecast_tomorrow).solution
            }
          ]
        }
      )
      assert page.has_content?('2 tests, 1 failure')
      click_button 'Add to tests suite'
      assert page.has_content?('3 tests, 0 failure')
    end
  end
end
