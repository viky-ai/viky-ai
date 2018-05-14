require 'test_helper'

class ChatStatementButtonTest < ActiveSupport::TestCase

  test 'Create a simple button statement' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button,
      content: {
        text: 'Self destruction',
        payload: {
          destruction: true,
          timer: '5 seconds'
        }
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
    assert !statement.component.is_disabled?
    assert !statement.component.is_selected?
  end


  test 'content blank validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button,
      content: {},
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'content text and payload validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button,
      content: {
        text: '',
        payload: {}
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.text can't be blank, content.payload can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content['text'] = 'a' * 101
    statement.content['payload'] = { a: 'a' }
    assert statement.invalid?
    expected = ['content.text is too long (maximum is 100 characters)']
    assert_equal expected, statement.errors.full_messages
  end


end
