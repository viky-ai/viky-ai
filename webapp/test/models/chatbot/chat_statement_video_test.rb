require 'test_helper'

class ChatStatementVideoTest < ActiveSupport::TestCase

  test 'Params presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :video,
      content: { params: '' },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.params can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Text length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :video,
      content: { params: 'a' * 5001 },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ['content.params is too long (maximum is 5000 characters)']
    assert_equal expected, statement.errors.full_messages
  end

end
