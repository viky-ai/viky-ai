require 'test_helper'

class FormulationsControllerTest < ActionDispatch::IntegrationTest
  #
  # Show
  #
  test 'Show forbidden' do
    sign_in users(:confirmed)
    get user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Show details
  #
  test 'Show formulation details forbidden' do
    sign_in users(:confirmed)
    get show_detailed_user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Create
  #
  test 'Create formulation access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_intent_formulations_url(users(:admin), agents(:weather), intents(:weather_forecast)),
         params: {
           formulation: { expression: 'Hello citizen' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create formulation forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_intent_formulations_url(users(:admin), agents(:weather), intents(:weather_forecast)),
         params: {
           formulation: { expression: 'Hello citizen' },
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Edit
  #
  test 'Edit formulation forbidden' do
    sign_in users(:show_on_agent_weather)
    get edit_user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Update
  #
  test 'Update formulation access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
          params: {
            formulation: { expression: 'Hello citizen' },
            format: :js
          }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Update formulation forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
          params: {
            formulation: { expression: 'Hello citizen' },
            format: :js
          }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Delete
  #
  test 'Delete formulation access' do
    sign_in users(:admin)
    delete user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
           params: {
             format: :js
           }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Delete formulation forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_intent_formulation_url(users(:admin), agents(:weather), intents(:weather_forecast), formulations(:weather_forecast_tomorrow)),
           params: {
             format: :js
           }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Scope agent on owner
  #
  test 'Formulation agent scoped on current user' do
    sign_in users(:confirmed)
    post user_agent_intent_formulations_url(users(:confirmed), agents(:weather_confirmed), intents(:weather_confirmed_question)),
         params: {
           formulation: { expression: 'Hello citizen' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end
end
