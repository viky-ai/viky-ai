require 'test_helper'

class InterpretationAliasTest < ActiveSupport::TestCase

  test 'Basic alias creation & interpretation association' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    weather_greeting_bonjour = interpretations(:weather_greeting_bonjour)
    interpretation_alias.interpretation = weather_greeting_bonjour

    weather_who = intents(:weather_who)
    interpretation_alias.intent = weather_who

    assert interpretation_alias.save
    assert_equal 8, interpretation_alias.position_start
    assert_equal 21, interpretation_alias.position_end
    assert_equal 'who', interpretation_alias.aliasname
    assert_equal weather_greeting_bonjour.id, interpretation_alias.interpretation.id
    assert_equal weather_who.id, interpretation_alias.intent.id
  end


  test 'Error on start position after end position' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 2,
      position_end: 1
    )
    interpretation_alias.interpretation = interpretations(:weather_greeting_bonjour)
    interpretation_alias.intent = intents(:weather_who)

    assert !interpretation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error on negative start and end positions' do
    interpretation_alias = InterpretationAlias.new(
      position_start: -9,
      position_end: -3
    )
    interpretation_alias.interpretation = interpretations(:weather_greeting_bonjour)
    interpretation_alias.intent = intents(:weather_who)

    assert !interpretation_alias.validate
    expected = ['Position start must be greater than or equal to 0', 'Position end must be greater than 0']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error start and end positions are egals' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 4,
      position_end: 4
    )
    interpretation_alias.interpretation = interpretations(:weather_greeting_bonjour)
    interpretation_alias.intent = intents(:weather_who)

    assert !interpretation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, interpretation_alias.errors.full_messages
  end
end
