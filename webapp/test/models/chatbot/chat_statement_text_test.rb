require 'test_helper'

class ChatStatementTextTest < ActiveSupport::TestCase

  test 'Text presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :text,
      content: { text: '' },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.text can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Text length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :text,
      content: { text: 'a' * 5001 },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ['content.text is too long (maximum is 5000 characters)']
    assert_equal expected, statement.errors.full_messages
  end


  test 'Speech Hash validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :text,
      content: {
        text: 'Hello',
        speech: ''
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.speech must be a Hash"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Speech text and locale validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :text,
      content: {
        text: 'Hello',
        speech: {
          text: '',
          locale: ''
        }
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.speech.text can't be blank, content.speech.locale is not included in the list"]
    assert_equal expected, statement.errors.full_messages

    statement.content = {
      text: 'Hello',
      speech: {
        text: 'Hi',
        locale: 'en-US'
      }
    }
    assert statement.valid?
  end

end
