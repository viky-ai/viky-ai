require 'test_helper'

class Chatbot::ListTest < ActiveSupport::TestCase

  test 'Create a list statement with default orientation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [
          {
            nature: 'text',
            content: {
              text: 'Hello'
            }
          },
          {
            nature: 'text',
            content: {
              text: 'How are you ?'
            }
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save

    assert_instance_of Chatbot::ChatStatementList, statement.component
    assert statement.component.is_horizontal?
    assert_not statement.component.is_vertival?
  end


  test 'Items size validation (too much)' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [{ nature: 'text', content: { text: 'Hello' } }] * 9
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = ["content.items is too long (maximum is 8 items)"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Items size validation (not enough)' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [{ nature: 'text', content: { text: 'Hello' } }]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.items is too short (minimum is 2 items)"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Items blank validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: []
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.items can't be blank, content.items is too short (minimum is 2 items)"]
    assert_equal expected, statement.errors.full_messages

    statement.content = { items: nil }
    assert statement.invalid?
    assert_equal expected, statement.errors.full_messages

    statement.content = {}
    assert statement.invalid?
    expected = ["content can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content = nil
    assert statement.invalid?
    expected = ["content can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Nested component validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [
          {
            nature: 'text',
            content: { text: '' }
          },
          {
            nature: 'text',
            content: { text: '' }
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      "Content item #0 content.text can't be blank, Content item #1 content.text can't be blank"
    ]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Orientation validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        orientation: :missing,
        items: [{ nature: 'text', content: { text: 'Hello' } }] * 3
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = ["content.orientation is not included in the list"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Nature validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [{ nature: 'foo bar' }]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.items is too short (minimum is 2 items), invalid nature, found: foo bar"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Speech validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :list,
      content: {
        items: [
          {
            nature: 'text',
            content: {
              text: 'Hello'
            }
          },
          {
            nature: 'text',
            content: {
              text: 'How are you ?'
            }
          }
        ],
        speech: {}
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.speech.text can't be blank, content.speech.locale is not included in the list"]
    assert_equal expected, statement.errors.full_messages
  end

end
