require 'test_helper'

class AgentsExportsControllerTest < ActionDispatch::IntegrationTest

  #
  # Download package
  #
  test 'Allow package download if collaborator (show)' do
    sign_in users(:show_on_agent_weather)
    get full_export_user_agent_url(users(:admin), agents(:weather), format: 'json')
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Allow package download if collaborator (edit)' do
    sign_in users(:edit_on_agent_weather)
    get full_export_user_agent_url(users(:admin), agents(:weather), format: 'json')
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Forbid package download with no access' do
    sign_in users(:confirmed)
    get full_export_user_agent_url(users(:admin), agents(:weather), format: 'json')
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end
end
