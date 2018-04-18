require 'test_helper'

class ChatSessionTest < ActiveSupport::TestCase

  test 'Create a simple ChatSession' do
    chat_session = ChatSession.new(
      user: users(:admin),
      bot: bots(:weather)
    )
    assert chat_session.save
  end
end
