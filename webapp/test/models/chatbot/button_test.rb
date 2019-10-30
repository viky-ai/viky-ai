require 'test_helper'

class Chatbot::ButtonTest < ActiveSupport::TestCase

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
    assert_not statement.component.is_disabled?
    assert_not statement.component.is_selected?
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
        payload: { a: 'a' }
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.text can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content['text'] = 'a' * 101
    assert statement.invalid?
    expected = ['content.text is too long (maximum is 100 characters)']
    assert_equal expected, statement.errors.full_messages

    statement.content['text'] = 'a'
    statement.content['payload'] = { a: 'a' }
    statement.content['href'] = 'callto:pseudoskype'
    assert statement.invalid?
    expected = ['content.href not allowed with payload']
    assert_equal expected, statement.errors.full_messages

    statement.content.delete('payload')
    statement.content['href'] = 'a' * 501
    assert statement.invalid?
    expected = ['content.href is too long (maximum is 500 characters)']
    assert_equal expected, statement.errors.full_messages

    statement.content['payload'] = { }
    statement.content['href'] = ''
    assert statement.invalid?
    expected = ["content payload and href can't both be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Allow specific href in button' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button,
      content: {
        text: 'Email me',
        href: 'mailto:someone@example.com?Subject=Hello%20again'
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end
end
