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
    get user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Show forbidden' do
    sign_in users(:confirmed)
    get user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
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
    assert_redirected_to user_agent_entities_lists_path(users(:admin), agents(:weather))
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
            origin: 'index',
            format: :json
          }
    assert_redirected_to user_agent_entities_lists_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Update proximity with access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
          params: {
            entities_list: { description: 'The new entities list name', proximity: 'far' },
            origin: 'show',
            format: :json
          }
    assert_redirected_to user_agent_entities_list_path(users(:admin), agents(:weather), entities_lists(:weather_conditions))
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


  #
  # Scope agent on owner
  #
  test 'Entities list agent scoped on current user' do
    sign_in users(:confirmed)
    post user_agent_entities_lists_url(users(:confirmed), agents(:weather_confirmed)),
         params: {
           entities_list: { listname: 'my_new_entities_list', description: 'A new entities list' },
           format: :json
         }
    assert_redirected_to user_agent_entities_lists_path(users(:confirmed), agents(:weather_confirmed))
    assert_nil flash[:alert]
  end


  #
  # Move intent to an other agent
  #
  test 'Allow to move an entities list' do
    sign_in users(:admin)

    post move_to_agent_user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           user: users(:admin).username,
           agent: agents(:terminator).agentname,
           format: :json
         }
    assert_redirected_to user_agent_entities_lists_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Forbid to move an entities list' do
    sign_in users(:confirmed)

    post move_to_agent_user_agent_entities_list_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           user: users(:admin).username,
           agent: agents(:weather_confirmed).agentname,
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end

end
