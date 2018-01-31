require 'test_helper'

class InterpretationsControllerTest < ActionDispatch::IntegrationTest
  #
  # Show
  #
  test 'Show forbidden' do
    sign_in users(:confirmed)
    get user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Show details
  #
  test 'Show interpretation details forbidden' do
    sign_in users(:confirmed)
    get show_detailed_user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Create
  #
  test 'Create interpretation access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_intent_interpretations_url(users(:admin), agents(:weather), intents(:weather_greeting)),
         params: {
           interpretation: { expression: 'Hello citizen' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create interpretation forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_intent_interpretations_url(users(:admin), agents(:weather), intents(:weather_greeting)),
         params: {
           interpretation: { expression: 'Hello citizen' },
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Edit
  #
  test 'Edit interpretation forbidden' do
    sign_in users(:show_on_agent_weather)
    get edit_user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Update
  #
  test 'Update interpretation access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
          params: {
            interpretation: { expression: 'Hello citizen' },
            format: :js
          }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Update interpretation forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
          params: {
            interpretation: { expression: 'Hello citizen' },
            format: :js
          }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Delete
  #
  test 'Delete interpretation access' do
    sign_in users(:admin)
    delete user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
           params: {
             format: :js
           }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Delete interpretation forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
           params: {
             format: :js
           }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end
end
