require 'test_helper'

class EntitiesListsExportsControllerTest < ActionDispatch::IntegrationTest

  test 'CSV access' do
    sign_in users(:show_on_agent_weather)
    get export_user_agent_entities_list_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions), format: :csv
    )
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'CSV forbidden' do
    sign_in users(:confirmed)
    get user_agent_entities_list_url(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    )
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
