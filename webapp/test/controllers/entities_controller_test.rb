require 'test_helper'

class InterpretationsControllerTest < ActionDispatch::IntegrationTest
  #
  # Create
  #
  test 'Create entity access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           entity: { terms: 'Hello' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create entity forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           entity: { terms: 'Hello' },
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Show
  #
  test 'Show entity forbidden' do
    sign_in users(:confirmed)
    get user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Show details
  #
  test 'Show entity details forbidden' do
    sign_in users(:confirmed)
    get show_detailed_user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end
end
