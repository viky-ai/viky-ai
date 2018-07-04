require 'test_helper'

class ChatStatementsControllerTest < ActionDispatch::IntegrationTest

  #
  # Index
  #
  test 'Index chat statements access' do
    sign_in users(:show_on_agent_weather)
    get chatbot_chat_statements_url(bots(:weather_bot), params: { format: :json })
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Index chat statements forbidden' do
    sign_in users(:confirmed)
    get chatbot_chat_statements_url(bots(:weather_bot), params: { format: :json })
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end

  #
  # Create
  #
  test 'Create chat statement access' do
    user = users(:show_on_agent_weather)
    bot = bots(:weather_bot)
    assert ChatSession.create(user: user, bot: bot)

    sign_in users(:show_on_agent_weather)
    post chatbot_chat_statements_url(bots(:weather_bot)),
      params: {
       statement: { content: 'Hello world' },
       format: :js
      }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create chat statement access if bot has WIP status and user can edit' do
    user = users(:edit_on_agent_weather)
    bot = bots(:weather_bot)
    bot.wip_enabled = true
    assert bot.save
    assert ChatSession.create(user: user, bot: bot)

    sign_in users(:edit_on_agent_weather)
    post chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           statement: { content: 'Hello world' },
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create chat statement forbidden if bot has WIP status' do
    bot = bots(:weather_bot)
    bot.wip_enabled = true
    assert bot.save

    sign_in users(:show_on_agent_weather)
    post chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           statement: { content: 'Hello world' },
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  test 'Create chat statement forbidden' do
    sign_in users(:confirmed)
    post chatbot_chat_statements_url(bots(:weather_bot)),
      params: {
        statement: { content: 'Hello world' },
       format: :js
      }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end


  #
  # User action
  #
  test 'User action chat statement access' do
    user = users(:show_on_agent_weather)
    bot = bots(:weather_bot)
    assert ChatSession.create(user: user, bot: bot)

    sign_in user
    post user_action_chatbot_chat_statements_url(bot),
         params: {
           payload: "foo bar",
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'User action chat statement forbidden if bot has WIP status' do
    bot = bots(:weather_bot)
    bot.wip_enabled = true
    assert bot.save

    sign_in users(:show_on_agent_weather)
    post user_action_chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           payload: "foo bar",
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  test 'User action chat statement forbidden' do
    sign_in users(:confirmed)
    post user_action_chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           payload: "foo bar",
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
