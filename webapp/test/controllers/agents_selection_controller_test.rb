require 'test_helper'

class AgentsSelectionControllerTest < ActionDispatch::IntegrationTest

  #
  # List available destinations
  #
  test 'Allow entities list available destinations' do
    sign_in users(:admin)

    get agents_selection_user_agent_url(users(:admin), agents(:weather), from: 'interpretations', current_id: interpretations(:weather_forecast).id)
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Forbid entities list available destinations' do
    sign_in users(:confirmed)

    get agents_selection_user_agent_url(users(:admin), agents(:weather), from: 'interpretations', current_id: interpretations(:weather_forecast).id)
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end
end
