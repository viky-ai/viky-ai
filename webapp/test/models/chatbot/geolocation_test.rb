require 'test_helper'

class Chatbot::GeolocationTest < ActiveSupport::TestCase

  test 'Geolocation creation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :geolocation,
      content: {
        text: 'Where are you ?'
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.valid?
  end


  test 'Geolocation content blank validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :geolocation,
      content: {},
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end
end
