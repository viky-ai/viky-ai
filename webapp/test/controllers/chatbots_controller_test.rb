require 'test_helper'

class ChatBotsControllerTest < ActionDispatch::IntegrationTest

  #
  # Feature chatbot control
  #
  test "Index bots access (feature chatbot disabled)" do
    Feature.with_chatbot_disabled do
      sign_in users(:admin)

      # Index
      assert_raise(ActionController::RoutingError) do
        get chatbots_url
      end

      # Show
      assert_raise(ActionController::RoutingError) do
        get chatbot_url(bots(:weather_bot))
      end
    end
  end

  test "Index bots access (feature chatbot enabled, user chatbot disabled)" do
    Feature.with_chatbot_enabled do
      # Index
      sign_in users(:admin)
      assert_raise(ActionController::RoutingError) do
        get chatbots_url
      end

      # Show
      sign_in users(:admin)
      assert_raise(ActionController::RoutingError) do
        get chatbot_url(bots(:weather_bot))
      end
    end
  end

  #
  # Index
  #
  test "Index bots access (feature chatbot enabled, user chatbot enabled)" do
    Feature.with_chatbot_enabled do
      users(:admin).update(chatbot_enabled: true)

      sign_in users(:admin)
      get chatbots_url
      assert_response :success
    end
  end

  #
  # Show
  #
  test 'Show chatbot access' do
    Feature.with_chatbot_enabled do
      users(:admin).update(chatbot_enabled: true)

      sign_in users(:admin)
      get chatbot_url(bots(:weather_bot))
      assert_response :success
      assert_nil flash[:alert]
    end
  end

  test 'Show chatbot forbidden' do
    Feature.with_chatbot_enabled do
      users(:confirmed).update(chatbot_enabled: true)

      sign_in users(:confirmed)
      get chatbot_url(bots(:weather_bot))
      assert_equal "Unauthorized operation.", flash[:alert]
    end
  end

  #
  # Reset
  #
  test 'Reset chatbot access' do
    Feature.with_chatbot_enabled do
      users(:admin).update(chatbot_enabled: true)

      sign_in users(:admin)
      get reset_chatbot_url(bots(:weather_bot))
      assert_redirected_to chatbot_path(bots(:weather_bot), recognition_locale: 'en-US')
      assert_nil flash[:alert]
    end
  end

  test 'Reset chatbot forbidden' do
    Feature.with_chatbot_enabled do
      users(:confirmed).update(chatbot_enabled: true)

      sign_in users(:confirmed)
      get reset_chatbot_url(bots(:weather_bot))
      assert_equal "Unauthorized operation.", flash[:alert]
    end
  end
end
