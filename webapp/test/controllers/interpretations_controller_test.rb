require 'test_helper'

class InterpretationsControllerTest < ActionDispatch::IntegrationTest
  #
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_intent_interpretations_url(users(:edit_on_agent_weather), agents(:weather), intents(:weather_greeting)),
         params: {
           interpretation: { expression: 'Hello citizen' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_intent_interpretations_url(users(:show_on_agent_weather), agents(:weather), intents(:weather_greeting)),
         params: {
           interpretation: { expression: 'Hello citizen' },
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end
end
