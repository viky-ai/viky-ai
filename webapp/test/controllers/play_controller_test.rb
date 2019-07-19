require 'test_helper'

class PlayControllerTest < ActionDispatch::IntegrationTest

  test 'interpret authorisation (edit)' do
    sign_in users(:edit_on_agent_weather)
    post "/play", params: {
      play_interpreter: {
        ownername: users(:admin).username,
        agentname: agents(:weather).agentname,
        text: "hello"
      }, format: :js
    }
    assert :success
  end


  test 'index authorisation (show)' do
    sign_in users(:show_on_agent_weather)
    post "/play", params: {
      play_interpreter: {
        ownername: users(:admin).username,
        agentname: agents(:weather).agentname,
        text: "hello"
      }, format: :js
    }
    assert :success
  end


  test 'index authorisation (forbidden)' do
    sign_in users(:confirmed)
    post "/play", params: {
      play_interpreter: {
        ownername: users(:admin).username,
        agentname: agents(:weather).agentname,
        text: "hello"
      }, format: :js
    }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  test 'index 404' do
    sign_in users(:edit_on_agent_weather)
    post "/play", params: {
      play_interpreter: {
        ownername: "missing",
        agentname: "missing",
        text: "hello"
      }, format: :js
    }
    assert_equal "302", response.code
    assert response.body.include? "404"
  end

end
