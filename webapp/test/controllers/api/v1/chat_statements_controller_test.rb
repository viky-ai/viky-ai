require 'test_helper'

class ChatStatementsControllerTest < ActionDispatch::IntegrationTest

  test 'Bot send an answer to a user statement' do
    post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
      statement: {
        nature: 'text',
        content: {
          text: 'Hello'
        }
      }
    }
    assert_equal '201', response.code
  end


  test 'Reject sending a statement to an old session' do
    assert ChatSession.create(
      user: users(:admin),
      bot: bots(:weather_bot)
    )
    post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
      statement: {
        nature: 'text',
        content: {
          text: 'Hello'
        }
      }
    }
    assert_equal '403', response.code
    assert_equal '{"errors":"Expired chat session"}', response.body
  end


  test 'Reject invalid statement' do
    post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
      statement: {
        nature: 'text',
        content: ''
      }
    }
    assert_equal '422', response.code
    assert_equal '{"errors":["Content can\'t be blank"]}', response.body
  end
end
