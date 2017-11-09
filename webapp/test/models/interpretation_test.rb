require 'test_helper'

class InterpretationTest < ActiveSupport::TestCase
  test 'Basic interpretation creation & intent association' do
    interpretation = Interpretation.new(expression: 'Good morning')
    interpretation.intent = intents(:weather_greeting)
    assert interpretation.save

    assert_equal 'Good morning', interpretation.expression
    assert_equal intents(:weather_greeting).id, interpretation.intent.id
    assert_equal 2, intents(:weather_greeting).interpretations.count
  end

  test 'Expression can\'t be blank and must be linked to an intent' do
    interpretation = Interpretation.new(expression: '')
    assert !interpretation.save

    expected = {
      intent: ['must exist'],
      expression: ['can\'t be blank']
    }
    assert_equal expected, interpretation.errors.messages
  end

  test 'Interpretation destroy' do
    interpretation = interpretations(:weather_greeting_hello)
    interpretation_id = interpretation.id

    assert_equal 1, Interpretation.where(id: interpretation_id).count
    assert interpretation.destroy
    assert_equal 0, Interpretation.where(id: interpretation_id).count
  end

end
