require 'test_helper'

class BotsControllerTest < ActionDispatch::IntegrationTest

  #
  # Index bots access
  #
  test "Index bots access" do
    sign_in users(:admin)
    get user_agent_bots_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Index bots forbidden" do
    sign_in users(:confirmed)
    get user_agent_bots_url(users(:admin), agents(:weather))
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_bots_url(users(:admin), agents(:weather)),
      params: {
       bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
       format: :json
      }
    assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Create forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_bots_url(users(:admin), agents(:weather)),
      params: {
       bot: { name: 'new bot', endpoint: 'http://www.new-bot.com/' },
       format: :json
      }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end


  #
  # Update
  #
  test 'Update access' do
    sign_in users(:edit_on_agent_weather)
    patch user_agent_bot_url(users(:admin), agents(:weather), bots(:weather)),
      params: {
        bot: { name: 'weather(updated)', endpoint: 'https://myweatherbot.com/api/updated' },
        format: :json
      }
    assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Update forbidden' do
    sign_in users(:show_on_agent_weather)
    patch user_agent_bot_url(users(:admin), agents(:weather), bots(:weather)),
      params: {
        bot: { name: 'weather(updated)', endpoint: 'https://myweatherbot.com/api/updated' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end


  #
  # Confirm_destroy access
  #
  test 'Confirm delete access' do
    sign_in users(:admin)
    get confirm_destroy_user_agent_bot_path(users(:admin), agents(:weather), bots(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Confirm delete forbidden' do
    sign_in users(:show_on_agent_weather)
    get confirm_destroy_user_agent_bot_path(users(:admin), agents(:weather), bots(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # Delete
  #
  test 'Delete access' do
    sign_in users(:admin)
    delete user_agent_bot_url(users(:admin), agents(:weather), bots(:weather))
    assert_redirected_to user_agent_bots_path(users(:admin), agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Delete forbidden' do
    sign_in users(:show_on_agent_weather)
    delete user_agent_bot_url(users(:admin), agents(:weather), bots(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
