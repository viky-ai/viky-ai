require 'test_helper'

class InterpretationsControllerTest < ActionDispatch::IntegrationTest

  #
  # Index interpretations access
  #
  test "Index interpretations access" do
    sign_in users(:admin)
    get user_agent_interpretations_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Index interpretations forbidden" do
    sign_in users(:confirmed)
    get user_agent_interpretations_url(users(:admin), agents(:weather))
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Show
  #
  test 'Show interpretation access' do
    sign_in users(:show_on_agent_weather)
    get user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Show interpretation forbidden' do
    sign_in users(:confirmed)
    get user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_interpretations_url(users(:admin), agents(:weather)),
         params: {
           interpretation: { interpretation_name: 'my_new_interpretation', description: 'A new interpretation' },
           format: :json
         }
    assert_redirected_to user_agent_interpretations_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Create forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_interpretations_url(users(:admin), agents(:weather)),
         params: {
           interpretation: { interpretation_name: 'my_new_interpretation', description: 'A new interpretation' },
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end


  #
  # Update
  #
  test 'Update access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast)),
          params: {
            interpretation: { interpretation_name: 'my_new_name', description: 'The new interpretation name' },
            format: :json
          }
    assert_redirected_to user_agent_interpretations_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Update forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast)),
          params: {
            interpretation: { interpretation_name: 'my_new_name', description: 'The new interpretation name' },
            format: :json
          }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end


  #
  # Confirm_destroy access
  #
  test 'Confirm delete access' do
    sign_in users(:admin)
    get user_agent_interpretation_confirm_destroy_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Confirm delete forbidden' do
    sign_in users(:show_on_agent_weather)
    get user_agent_interpretation_confirm_destroy_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Delete
  #
  test 'Delete access' do
    sign_in users(:admin)
    delete user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_redirected_to user_agent_interpretations_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Delete forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Add locale
  #
  test 'Add locale' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_interpretation_add_locale_url(users(:admin), agents(:weather), interpretations(:weather_forecast), locale_to_add: 'fr')
    assert_redirected_to user_agent_interpretation_path(users(:admin), agents(:weather), interpretations(:weather_forecast), locale: 'fr')
    assert_nil flash[:alert]
  end

  test 'Add locale forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_interpretation_add_locale_url(users(:admin), agents(:weather), interpretations(:weather_forecast), locale_to_add: 'fr')
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Scope agent on owner
  #
  test 'interpretation agent scoped on current user' do
    sign_in users(:confirmed)
    post user_agent_interpretations_url(users(:confirmed), agents(:weather_confirmed)),
         params: {
           interpretation: { interpretation_name: 'my_new_interpretation', description: 'A new interpretation' },
           format: :json
         }
    assert_redirected_to user_agent_interpretations_path(users(:confirmed), agents(:weather_confirmed))
    assert_nil flash[:alert]
  end


  #
  # Move interpretation to an other agent
  #
  test 'Allow to move an interpretation' do
    sign_in users(:admin)

    post move_to_agent_user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast)),
         params: {
           user: users(:admin).username,
           agent: agents(:terminator).agentname,
           format: :json
         }
    assert_redirected_to user_agent_interpretations_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Forbid to move an interpretation' do
    sign_in users(:confirmed)

    post move_to_agent_user_agent_interpretation_url(users(:admin), agents(:weather), interpretations(:weather_forecast)),
         params: {
           user: users(:admin).username,
           agent: agents(:weather_confirmed).agentname,
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end
end
