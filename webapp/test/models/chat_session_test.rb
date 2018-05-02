require 'test_helper'

class ChatSessionTest < ActiveSupport::TestCase

  test 'Create a simple ChatSession' do
    chat_session = ChatSession.new(
      user: users(:admin),
      bot: bots(:weather_bot)
    )
    assert chat_session.save
  end


  test 'Check expired chat session' do
    old_session = chat_sessions(:one)
    assert ChatSession.create(
      user: users(:admin),
      bot: bots(:terminator_bot)
    )
    assert !old_session.expired?

    assert ChatSession.create(
      user: users(:admin),
      bot: bots(:weather_bot)
    )
    assert old_session.expired?
  end
end
