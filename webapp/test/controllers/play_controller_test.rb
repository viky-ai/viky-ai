require 'test_helper'

class PlayControllerTest < ActionDispatch::IntegrationTest

  test 'index authorisation (edit)' do
    sign_in users(:edit_on_agent_weather)
    get "/play/#{users(:admin).username}/#{agents(:weather).agentname}/"
    assert :success
    assert_nil flash[:alert]
  end


  test 'index authorisation (show)' do
    sign_in users(:show_on_agent_weather)
    get "/play/#{users(:admin).username}/#{agents(:weather).agentname}/"
    assert :success
    assert_nil flash[:alert]
  end


  test 'index authorisation (forbidden)' do
    sign_in users(:confirmed)
    get "/play/#{users(:admin).username}/#{agents(:weather).agentname}/"
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  test 'index 404' do
    sign_in users(:edit_on_agent_weather)
    get "/play/missing/missing/"
    assert_equal "302", response.code
    assert response.body.include? "404"
  end

end
