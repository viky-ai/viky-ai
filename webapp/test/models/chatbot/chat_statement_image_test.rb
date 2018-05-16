require 'test_helper'

class ChatStatementImageTest < ActiveSupport::TestCase

  test 'Create a simple image statement' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :image,
      content: {
        url: 'https://www.pertimm.com/assets/img/logo_pertimm.png',
        title: 'Pertimm',
        description: 'The Pertimm logo.'
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'image url presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :image,
      content: {
        url: '',
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    assert_equal ["content.url can't be blank"], statement.errors.full_messages
  end


  test 'image url, title and description length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :image,
      content: {
        url: 'a' * 5001,
        title: 'a' * 101,
        description: 'a' * 501
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = [
      'content.url is too long (maximum is 5000 characters)',
      'content.title is too long (maximum is 100 characters)',
      'content.description is too long (maximum is 500 characters)',
    ].join(', ')

    assert_equal [expected], statement.errors.full_messages
  end

end
