require 'test_helper'

class Chatbot::ButtonGroupTest < ActiveSupport::TestCase

  test 'Button group creation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        buttons: [{
          text: 'I am a button',
          payload: { action: "Go"}
        }]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.valid?
  end


  test 'Nested buttons presence validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        buttons: []
      },
      chat_session: chat_sessions(:one)
    )

    assert statement.invalid?
    expected = ["content.buttons can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content = { buttons: nil }
    expected = ["content.buttons can't be blank"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Nested buttons length validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        buttons: [
          {
            text: 'I am a button',
            payload: { action: "Go"}
          }
        ] * 7
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["content.buttons is too long (maximum is 6 buttons)"]
    assert_equal expected, statement.errors.full_messages
  end


  test 'Nested button attributes validation' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        buttons: [
          {
            text: '',
            payload: {}
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = ["Content button #0 content.text can't be blank, content.payload can't be blank"]
    assert_equal expected, statement.errors.full_messages

    statement.content = {
      buttons: [
        {
          text: '',
          payload: []
        }
      ]
    }
    assert statement.invalid?
    assert_equal expected, statement.errors.full_messages

    statement.content = {
      buttons: [
        {
          text: '',
          payload: nil
        }
      ]
    }
    assert statement.invalid?
    assert_equal expected, statement.errors.full_messages

    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        buttons: [
          {
            text: 'Hi',
            payload: {}
          },
          {
            text: '',
            payload: { action: 'Go'}
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.invalid?
    expected = [
      "Content button #0 content.payload can't be blank, Content button #1 content.text can't be blank"
    ]
    assert_equal expected, statement.errors.full_messages
  end


  test 'button group statement disable' do
    statement = ChatStatement.new(
      speaker: :bot,
      nature: :button_group,
      content: {
        disable_on_click: true,
        buttons: [
          {
            text: 'Button A',
            payload: { foo: :bar }
          },
          {
            text: 'Button B',
            payload: { foo: :bar }
          }
        ]
      },
      chat_session: chat_sessions(:one)
    )
    assert statement.save
    assert statement.component.disable(statement, 1)
    assert_equal [true, true], statement.component.buttons_as_components.collect(&:disabled)
    assert_equal [false, true], statement.component.buttons_as_components.collect(&:selected)
  end

end
