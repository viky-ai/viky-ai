require 'test_helper'

class Chatbot::MapTest < ActiveSupport::TestCase

  test 'Create a simple map statement' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :map,
      content: {
        params: "place?key=***REMOVED***&q=Valence"
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'Params presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :map,
      content: { params: '' },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.params can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Params, title and description length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :map,
      content: {
        params: 'a' * 5001,
        title: 'a' * 101,
        description: 'a' * 501
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = [
      'content.params is too long (maximum is 5000 characters)',
      'content.title is too long (maximum is 100 characters)',
      'content.description is too long (maximum is 500 characters)',
    ].join(', ')

    assert_equal [expected], statement.errors.full_messages
  end

end
