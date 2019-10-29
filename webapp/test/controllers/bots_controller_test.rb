require 'test_helper'

class BotsControllerTest < ActionDispatch::IntegrationTest

  #
  # Feature chatbot control
  #
  test "access (feature chatbots disabled)" do
    Feature.with_chatbot_disabled do
      sign_in users(:admin)

      # index
      assert_raise(ActionController::RoutingError) do
        get user_agent_bots_url(users(:admin), agents(:weather))
      end

      # create
      assert_raise(ActionController::RoutingError) do
        post user_agent_bots_url(users(:admin), agents(:weather)),
          params: {
           bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
           format: :json
          }
      end
    end
  end

  test "access (feature chatbots enabled, user chatbot disabled)" do
    Feature.with_chatbot_enabled do
      # index
      sign_in users(:admin)
      assert_raise(ActionController::RoutingError) do
        get user_agent_bots_url(users(:admin), agents(:weather))
      end

      # create
      sign_in users(:admin)
      assert_raise(ActionController::RoutingError) do
        post user_agent_bots_url(users(:admin), agents(:weather)),
          params: {
           bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
           format: :json
          }
      end
    end
  end


  #
  # Index bots access
  #
  test "Index bots access" do
    users(:admin).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:admin)
      get user_agent_bots_url(users(:admin), agents(:weather))
      assert_response :success
      assert_nil flash[:alert]
    end
  end

  test "Index bots forbidden" do
    users(:confirmed).chatbot_enabled = true
    users(:confirmed).save

    Feature.with_chatbot_enabled do
      sign_in users(:confirmed)
      get user_agent_bots_url(users(:admin), agents(:weather))
      assert_equal "Unauthorized operation.", flash[:alert]
    end
  end


  #
  # Create
  #
  test 'Create access' do
    users(:edit_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:edit_on_agent_weather)
      post user_agent_bots_url(users(:admin), agents(:weather)),
        params: {
         bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
         format: :json
        }
      assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
      assert_nil flash[:alert]
    end
  end

  test 'Create forbidden' do
    users(:show_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:show_on_agent_weather)
      post user_agent_bots_url(users(:admin), agents(:weather)),
        params: {
         bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
         format: :json
        }
      assert_response :forbidden
      assert response.body.include?('Unauthorized operation.')
    end
  end


  #
  # Update
  #
  test 'Update access' do
    users(:edit_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:edit_on_agent_weather)
      patch user_agent_bot_url(users(:admin), agents(:weather), bots(:weather_bot)),
        params: {
          bot: { name: 'weather(updated)', endpoint: 'https://myweatherbot.com/api/updated' },
          format: :json
        }
      assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
      assert_nil flash[:alert]
    end
  end

  test 'Update forbidden' do
    users(:show_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:show_on_agent_weather)
      patch user_agent_bot_url(users(:admin), agents(:weather), bots(:weather_bot)),
        params: {
          bot: { name: 'weather(updated)', endpoint: 'https://myweatherbot.com/api/updated' },
          format: :json
        }
      assert_response :forbidden
      assert response.body.include?('Unauthorized operation.')
    end
  end


  #
  # Confirm_destroy access
  #
  test 'Confirm delete access' do
    users(:admin).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:admin)
      get confirm_destroy_user_agent_bot_path(users(:admin), agents(:weather), bots(:weather_bot))
      assert_response :success
      assert_nil flash[:alert]
    end
  end

  test 'Confirm delete forbidden' do
    users(:show_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:show_on_agent_weather)
      get confirm_destroy_user_agent_bot_path(users(:admin), agents(:weather), bots(:weather_bot))
      assert_redirected_to agents_url
      assert_equal 'Unauthorized operation.', flash[:alert]
    end
  end


  #
  # Delete
  #
  test 'Delete access' do
    users(:admin).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:admin)
      delete user_agent_bot_url(users(:admin), agents(:weather), bots(:weather_bot))
      assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
      assert_nil flash[:alert]
    end
  end

  test 'Delete forbidden' do
    users(:show_on_agent_weather).update(chatbot_enabled: true)

    Feature.with_chatbot_enabled do
      sign_in users(:show_on_agent_weather)
      delete user_agent_bot_url(users(:admin), agents(:weather), bots(:weather_bot))
      assert_redirected_to agents_url
      assert_equal 'Unauthorized operation.', flash[:alert]
    end
  end

end
