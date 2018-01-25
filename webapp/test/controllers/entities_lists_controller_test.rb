require 'test_helper'

class EntitiesListsControllerTest < ActionDispatch::IntegrationTest
  #
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_entities_lists_url(users(:admin), agents(:weather)),
         params: {
           entities_list: { listname: 'my_new_entities_list', description: 'A new entities list' },
           format: :json
         }
    assert_redirected_to user_agent_url(users(:edit_on_agent_weather), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Create forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_entities_lists_url(users(:admin), agents(:weather)),
         params: {
           entities_list: { listname: 'my_new_entities_list', description: 'A new entities list' },
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
    patch entities_list_url(entities_lists(:weather_conditions)),
          params: {
            entities_list: { listname: 'my_new_name', description: 'The new entities list name' },
            format: :json
          }
    assert_redirected_to user_agent_url(users(:edit_on_agent_weather), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Update forbidden' do
    sign_in users(:show_on_agent_weather)
    patch entities_list_url(entities_lists(:weather_conditions)),
          params: {
            entities_list: { listname: 'my_new_name', description: 'The new entities list name' },
            format: :json
          }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end
end
