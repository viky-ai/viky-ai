require 'test_helper'

class EntitiesListsControllerTest < ActionDispatch::IntegrationTest

  #
  # Index entities lists access
  #
  test "Index entities lists access" do
    sign_in users(:admin)
    get user_agent_entities_lists_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Index entities lists forbidden" do
    sign_in users(:confirmed)
    get user_agent_entities_lists_url(users(:admin), agents(:weather))
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  #
  # Show
  #
  test 'Show access' do
    sign_in users(:show_on_agent_weather)
    get user_agent_entities_list_url(users(:show_on_agent_weather), agents(:weather), entities_lists(:weather_conditions))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Show forbidden' do
    sign_in users(:confirmed)
    get user_agent_entities_list_url(users(:confirmed), agents(:terminator), entities_lists(:weather_conditions))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


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
    assert_redirected_to user_agent_entities_lists_path(users(:edit_on_agent_weather), agents(:weather))
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
    patch user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
          params: {
            entities_list: { listname: 'my_new_name', description: 'The new entities list name' },
            format: :json
          }
    assert_redirected_to user_agent_entities_lists_path(users(:edit_on_agent_weather), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Update forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
          params: {
            entities_list: { listname: 'my_new_name', description: 'The new entities list name' },
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
    get user_agent_entities_list_confirm_destroy_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Confirm delete forbidden' do
    sign_in users(:show_on_agent_weather)
    get user_agent_entities_list_confirm_destroy_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Delete
  #
  test 'Delete access' do
    sign_in users(:admin)
    delete user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_redirected_to user_agent_entities_lists_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Delete forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
