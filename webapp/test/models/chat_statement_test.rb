require 'test_helper'

class ChatStatementTest < ActiveSupport::TestCase

  test 'Create a simple chat statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: { text: 'Good morning !' },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
  end


  test 'Validate chat statement' do
    statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:text],
      content: '',
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      "content can't be blank",
    ]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Create a simple notification statement' do
    statement_notification = ChatStatement.new(
      speaker: ChatStatement.speakers[:moderator],
      nature: ChatStatement.natures[:notification],
      content: {
        text: 'Bot error',
      },
      chat_session: chat_sessions(:one)
    )
    assert statement_notification.save
  end


  test 'Validate notification statement' do
    statement_notification = ChatStatement.new(
      speaker: ChatStatement.speakers[:moderator],
      nature: ChatStatement.natures[:notification],
      content: {
        text: ''
      },
      chat_session: chat_sessions(:one)
    )
    assert statement_notification.invalid?
    assert_equal ["Text can't be blank"], statement_notification.errors.full_messages

    statement_notification.content['text'] = 'a' * 2001
    assert statement_notification.invalid?
    assert_equal ['Text is too long (maximum is 2000 characters)'], statement_notification.errors.full_messages
  end


  test 'Create a simple card statement' do
    card_statement = ChatStatement.new(
      speaker: ChatStatement.speakers[:bot],
      nature: ChatStatement.natures[:card],
      content: {
        components: [{
          nature: 'image',
          content: {
            url: 'https://www.pertimm.com/assets/img/logo_pertimm.png'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'button',
          content: {
            text: 'Fire !',
            payload: { foo: :bar }
          }
        }, {
          nature: 'image',
          content: {
            url: 'https://www.pertimm.com/assets/img/logo_pertimm.png'
          }
        }],
        speech: {
          text: 'Pertimm FTW !',
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
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }, {
          nature: 'text',
          content: {
            text: 'The Pertimm logo.'
          }
        }],
      },
      chat_session: chat_sessions(:one)
    )

    assert card_statement.invalid?
    assert_equal ['Components is too long (maximum is 5 items)'], card_statement.errors.full_messages

    card_statement.content = { components: [] }
    assert card_statement.invalid?
    assert_equal ["Components can't be blank"], card_statement.errors.full_messages

    card_statement.content = { components: [{
      nature: 'foobar',
      content: {
        text: ''
      }
    }] }
    assert card_statement.invalid?
    assert_equal ['invalid nature, found: foobar'], card_statement.errors.full_messages

    card_statement.content = { components: [{
      nature: 'image',
      content: {
        url: ''
      }
    }] }
    assert card_statement.invalid?
    assert_equal ["content.url can't be blank"], card_statement.errors.full_messages


    card_statement.content = {
      components: [{
        nature: :button_group,
        content: {
          buttons: [{
            text: 'Button 1',
            payload: { button: 1 }
          }]
        }
      }],
      payload: { data: :misc }
    }
    assert card_statement.invalid?
    assert_equal ["Card payload with button or button group."], card_statement.errors.full_messages
  end
end
