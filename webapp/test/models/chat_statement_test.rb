require 'test_helper'

class ChatStatementTest < ActiveSupport::TestCase

  test 'Create a simple chat statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: { text: 'Good morning !' },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'Validate chat statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: '',
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      "Content can't be blank",
    ]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Validate text statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: { text: '' },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      "Text can't be blank"
    ]
    assert_equal expected, statement.errors.full_messages

    statement.content = { text: 'a' * 5001 }
    assert statement.invalid?
    assert_equal ['Text is too long (maximum is 5000 characters)'], statement.errors.full_messages
  end


  test 'Create a simple list statement' do
    statement_list = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:list],
      content: {
        list: [
          {
            nature: 'text',
            text: 'Hello'
          },
          {
            nature: 'text',
            text: 'How are you ?'
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement_list.save
  end


  test 'Validate a list statement' do
    statement_list = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:list],
      content: {
        list: [
          {
            nature: 'text',
            text: 'Hello'
          },
          {
            nature: 'text',
            text: 'How are you ?'
          },
          {
            nature: 'text',
            text: 'What are you doing ?'
          },
          {
            nature: 'text',
            text: 'What is your name ?'
          },
          {
            nature: 'text',
            text: 'TALK TO ME !!!'
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement_list.invalid?
    assert_equal ['List is too long (maximum is 4 items)'], statement_list.errors.full_messages

    statement_list.content = { list: [] }
    assert statement_list.invalid?
    assert_equal ["List can't be blank"], statement_list.errors.full_messages

    statement_list.content = {
      list: [{
        list: []
      }]
    }
    assert statement_list.invalid?
    assert_equal ['invalid nature'], statement_list.errors.full_messages

    statement_list.content = {
      list: [
        {
          nature: 'text',
          text: ''
        }
      ]
    }
    assert statement_list.invalid?
    assert_equal ["Text can't be blank"], statement_list.errors.full_messages
  end
end
