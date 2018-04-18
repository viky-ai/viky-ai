require 'test_helper'

class ChatStatementTest < ActiveSupport::TestCase

  test 'Create a simple chat statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: 'Good morning !',
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end
end
