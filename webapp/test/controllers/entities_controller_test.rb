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
  # Edit
  #
  test 'Edit entity forbidden' do
    sign_in users(:show_on_agent_weather)
    get edit_user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
        params: {
          format: :js
        }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Update
  #
  test 'Update entity access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
          params: {
            entity: { terms: 'Sunshine' },
            format: :js
          }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Update entity forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
          params: {
            entity: { terms: 'Sunshine' },
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

  #
  # Delete
  #
  test 'Delete access entity' do
    sign_in users(:admin)
    delete user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
           params: {
             format: :js
           }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Delete forbidden entity' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_entities_list_entity_url(users(:admin), agents(:weather), entities_lists(:weather_conditions), entities(:weather_sunny)),
           params: {
             format: :js
           }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Update position
  #
  test 'Update entity position allowed' do
    sign_in users(:admin)
    post update_positions_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           ids: [entities(:weather_raining).id, entities(:weather_sunny).id],
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Update entity position forbidden' do
    sign_in users(:show_on_agent_weather)
    post update_positions_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           ids: [entities(:weather_raining).id, entities(:weather_sunny).id],
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Select import entities lists access
  #
  test "Select import entities lists access" do
    sign_in users(:admin)
    get new_import_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Select import entities lists forbidden" do
    sign_in users(:confirmed)
    get new_import_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Import entities lists access
  #
  test "Import entities lists access" do
    sign_in users(:admin)
    post create_import_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           import: {
             file: fixture_file_upload('files/import_entities.csv', 'text/csv'),
             mode: 'replace'
           },
           format: :json
         }
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Import entities lists access-failed" do
    sign_in users(:admin)
    post create_import_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           import: {
             file: fixture_file_upload('files/import_entities.csv', 'text/csv')
           },
           format: :json
         }
    assert_response :unprocessable_entity
  end

  test "Import entities lists forbidden" do
    sign_in users(:confirmed)
    post create_import_user_agent_entities_list_entities_url(users(:admin), agents(:weather), entities_lists(:weather_conditions)),
         params: {
           import: {
             file: fixture_file_upload('files/import_entities.csv', 'text/csv'),
             mode: 'replace'
           },
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end



  #
  # Scope agent on owner
  #
  test 'Entity agent scoped on current user' do
    sign_in users(:confirmed)
    post user_agent_entities_list_entities_url(users(:confirmed), agents(:weather_confirmed), entities_lists(:weather_confirmed_dates)),
         params: {
           entity: { terms: 'Hello' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end
end
