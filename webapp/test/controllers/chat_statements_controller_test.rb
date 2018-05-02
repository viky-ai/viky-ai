require 'test_helper'

class ChatStatementsControllerTest < ActionDispatch::IntegrationTest

  #
  # Create
  #
  test 'Create access chat statement' do
    sign_in users(:admin)
    post chatbot_chat_statements_url(bots(:weather_bot)),
      params: {
       statement: { content: 'Hello world' },
       format: :js
      }
    assert :success
    assert_nil flash[:alert]
  end

  test 'Create forbidden chat statement' do
    sign_in users(:show_on_agent_weather)
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
  test 'User action access chat statement' do
    sign_in users(:admin)
    post user_action_chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           payload: "foo bar",
           format: :js
         }
    assert :success
    assert_nil flash[:alert]
  end

  test 'User action forbidden chat statement' do
    sign_in users(:show_on_agent_weather)
    post user_action_chatbot_chat_statements_url(bots(:weather_bot)),
         params: {
           payload: "foo bar",
           format: :js
         }
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
