require 'test_helper'

class ChatSessionsControllerTest < ActionDispatch::IntegrationTest

  test 'Bot update locale' do
    assert_equal 'en-US', chat_sessions(:one).locale
    put "/api/v1/chat_sessions/#{chat_sessions(:one).id}.json", params: {
      chat_session: {
        locale: 'fr-FR'
      }
    }
    assert_equal '200', response.code
    assert_equal 'fr-FR', chat_sessions(:one).reload.locale
  end


  test 'Bot failed to update locale' do
    assert_equal 'en-US', chat_sessions(:one).locale
    put "/api/v1/chat_sessions/#{chat_sessions(:one).id}.json", params: {
      chat_session: {
        locale: 'missing'
      }
    }
    assert_equal '422', response.code
    expected = {
      "errors" => ["Locale is not included in the list"]
    }
    assert_equal expected, JSON.parse(response.body)
    assert_equal 'en-US', chat_sessions(:one).reload.locale
  end

end
