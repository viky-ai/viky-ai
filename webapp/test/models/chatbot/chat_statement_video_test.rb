require 'test_helper'

class ChatStatementVideoTest < ActiveSupport::TestCase

  test 'Create a simple video statement' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :video,
      content: {
        params: 'video_id'
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


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


  test 'Params, title and subtitle length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :video,
      content: {
        params: 'a' * 5001,
        title: 'a' * 101,
        subtitle: 'a' * 501
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = [
      'content.params is too long (maximum is 5000 characters)',
      'content.title is too long (maximum is 100 characters)',
      'content.subtitle is too long (maximum is 500 characters)',
    ].join(', ')

    assert_equal [expected], statement.errors.full_messages
  end
end
