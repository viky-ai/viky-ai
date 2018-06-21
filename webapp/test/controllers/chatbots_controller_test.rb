require 'test_helper'

class ChatBotsControllerTest < ActionDispatch::IntegrationTest


  test 'Show chatbot access' do
    sign_in users(:admin)
    get chatbot_url(bots(:weather_bot))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Show chatbot forbidden' do
    sign_in users(:confirmed)
    get chatbot_url(bots(:weather_bot))
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test 'Reset chatbot access' do
    sign_in users(:admin)
    get reset_chatbot_url(bots(:weather_bot))
    assert_redirected_to chatbot_path(bots(:weather_bot), recognition_locale: 'en-US')
    assert_nil flash[:alert]
  end

  test 'Reset chatbot forbidden' do
    sign_in users(:confirmed)
    get reset_chatbot_url(bots(:weather_bot))
    assert_equal "Unauthorized operation.", flash[:alert]
  end
end
