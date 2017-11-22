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
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_intent_interpretations_url(users(:admin), agents(:weather), intents(:weather_greeting)),
         params: {
           interpretation: { expression: 'Hello citizen' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create forbidden' do
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
  test 'Edit forbidden' do
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
  test 'Update access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
          params: {
            interpretation: { expression: 'Hello citizen' },
            format: :js
          }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Update forbidden' do
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
  test 'Delete access' do
    sign_in users(:admin)
    delete user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
           params: {
             format: :js
           }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Delete forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_intent_interpretation_url(users(:admin), agents(:weather), intents(:weather_greeting), interpretations(:weather_greeting_hello)),
           params: {
             format: :js
           }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end
end
