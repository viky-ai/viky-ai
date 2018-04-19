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
      'Content text key missing'
    ]
    assert_equal expected, statement.errors.full_messages

    statement.content = { text: 'a' * 5001 }
    assert statement.invalid?
    assert_equal ['Content is too long (maximum is 5000 characters)'], statement.errors.full_messages
  end


  test 'Validate text statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: { words: 'Good morning !' },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      'Content text key missing'
    ]
    assert_equal expected, statement.errors.full_messages

    statement.content = { text: '' }
    assert statement.invalid?
    expected = [
      "Content text can't be blank"
    ]
    assert_equal expected, statement.errors.full_messages
  end
end
