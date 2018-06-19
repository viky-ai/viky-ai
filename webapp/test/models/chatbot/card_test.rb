require 'test_helper'

class Chatbot::CardTest < ActiveSupport::TestCase

  test 'Create a simple card statement' do
    card_statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:card],
      content: {
        components: [{
          nature:  'image',
          content: {
            url: 'https://www.pertimm.com/assets/img/logo_pertimm.png'
          }
        }, {
          nature:  'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature:  'button',
          content: {
            text:    'Fire !',
            payload: { foo: :bar }
          }
        }, {
          nature:  'image',
          content: {
            url: 'https://www.pertimm.com/assets/img/logo_pertimm.png'
          }
        }],
        speech:     {
          text:   'Pertimm FTW !',
          locale: 'en-US'
        }
      },
      chat_session: chat_sessions(:one)
    )

    assert card_statement.save
  end


  test 'Validate card statement' do
    card_statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:card],
      content: {
        components: [{
          nature:  'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }] * 7
      },
      chat_session: chat_sessions(:one)
    )

    assert card_statement.invalid?
    assert_equal ['Components is too long (maximum is 6 items)'], card_statement.errors.full_messages

    card_statement.content = { components: [] }
    assert card_statement.invalid?
    assert_equal ["Components can't be blank"], card_statement.errors.full_messages

    card_statement.content = { components: [{
      nature:  'foobar',
      content: {
        text: ''
      }
    }] }
    assert card_statement.invalid?
    assert_equal ['invalid nature, found: foobar'], card_statement.errors.full_messages

    card_statement.content = { components: [{
      nature:  'image',
      content: {
        url: ''
      }
    }] }
    assert card_statement.invalid?
    assert_equal ["content.url can't be blank"], card_statement.errors.full_messages
  end

end
