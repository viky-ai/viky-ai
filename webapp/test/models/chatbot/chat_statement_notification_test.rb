require 'test_helper'

class ChatStatementNotificationTest < ActiveSupport::TestCase

  test 'Create a simple notification statement' do
    statement = ChatStatement.new(
      speaker: :moderator,
      nature: :notification,
      content: {
        text: 'Bot error',
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'text validation' do
    statement = ChatStatement.new(
      speaker: :moderator,
      nature: :notification,
      content: {
        text: ''
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.text can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content['text'] = 'a' * 2001
    assert statement.invalid?
    expected = ['content.text is too long (maximum is 2000 characters)']
    assert_equal expected, statement.errors.full_messages
  end

end
