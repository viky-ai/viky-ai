require 'test_helper'

class PlayControllerTest < ActionDispatch::IntegrationTest

  test 'interpret authorisation (edit)' do
    sign_in users(:edit_on_agent_weather)
    post "/play", params: {
      play_interpreter: {
        agent_id: agents(:weather).id,
        text: "hello"
      }, format: :js
    }
    assert :success
  end


  test 'index authorisation (show)' do
    sign_in users(:show_on_agent_weather)
    post "/play", params: {
      play_interpreter: {
        agent_id: agents(:weather).id,
        text: "hello"
      }, format: :js
    }
    assert :success
  end


  test 'index authorisation (forbidden)' do
    sign_in users(:confirmed)
    post "/play", params: {
      play_interpreter: {
        agent_id: agents(:weather).id,
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
        agent_id: "123",
        text: "hello"
      }, format: :js
    }
    assert_equal "302", response.code
    assert response.body.include? "404"

    post "/play", params: {
      play_interpreter: {
        text: "hello"
      }, format: :js
    }
    assert_equal "302", response.code
    assert response.body.include? "404"
  end

end
