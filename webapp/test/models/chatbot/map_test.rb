require 'test_helper'

class Chatbot::MapTest < ActiveSupport::TestCase

  test 'Create a simple map statement' do
    google_api_key = ENV.fetch('VIKYAPP_GOOGLE_MAP_API_KEY') { 'missing-google-map-api-key' }
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :interactive_map,
      content: {
        params: "place?key=#{google_api_key}&q=Valence"
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'Params presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :interactive_map,
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
      nature: :interactive_map,
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
