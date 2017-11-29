require 'test_helper'

class InterpretationAliasTest < ActiveSupport::TestCase

  test 'remove intent used as alias remove related interpretation alias' do
    weather_who_intent = intents(:weather_who)
    interpretation = interpretations(:weather_greeting_hello)

    assert_equal weather_who_intent.id, interpretation.interpretation_aliases.first.intent.id

    assert weather_who_intent.destroy
    assert_equal 0, interpretation.interpretation_aliases.count
  end


  test 'intent_id loop reference' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who',
      interpretation_id: interpretations(:weather_greeting_bonjour).id,
      intent_id: intents(:weather_greeting).id
    )

    assert !interpretation_alias.save
    assert_equal ["Intent can't be interpretation intent"], interpretation_alias.errors.full_messages
  end


  test 'aliases no overlap' do
    # Create with 1 range
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_greeting).id,
      locale: 'fr-FR',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert interpretation.save

    # Create with 2 ranges without overlap
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_greeting).id,
      locale: 'fr-FR',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who1',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who2',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert interpretation.save

    # Create with 2 ranges with overlap
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_greeting).id,
      locale: 'fr-FR',
      interpretation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert !interpretation.save
    assert_equal ["Interpretation aliases position overlap"], interpretation.errors.full_messages

    # Update with 2 ranges with overlap
    assert !interpretation.update({
      interpretation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert_equal ["Interpretation aliases position overlap"], interpretation.errors.full_messages


    # Update with 3 ranges
    assert !interpretation.update({
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who3',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who4',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 8,
          position_end: 12,
          aliasname: 'who5',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert_equal ["Interpretation aliases position overlap"], interpretation.errors.full_messages
  end


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


  test 'Aliasname must be present' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      interpretation: interpretations(:weather_greeting_bonjour),
      intent: intents(:weather_who)
    )
    assert !interpretation_alias.save
    expected = ['Parameter name can\'t be blank', 'Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error on start position after end position' do
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
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
      aliasname: 'who',
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
      aliasname: 'who',
      position_start: 4,
      position_end: 4
    )
    interpretation_alias.interpretation = interpretations(:weather_greeting_bonjour)
    interpretation_alias.intent = intents(:weather_who)

    assert !interpretation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Check aliasname uniqueness' do
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_greeting).id,
      locale: 'fr-FR',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 11,
          aliasname: 'who',
          intent_id: intents(:weather_who).id
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert !interpretation.save
    expected = ['Interpretation aliases aliasname has already been taken']
    assert_equal expected, interpretation.errors.full_messages
  end


  test 'Redefine deleted aliasname' do
    interpretation = interpretations(:weather_greeting_hello)
    interpretation.update({
      interpretation_aliases_attributes: [
        {
          id: interpretation_aliases(:weather_greeting_hello_who).id,
          position_start: 6,
          position_end: 11,
          aliasname: 'who',
          intent_id: intents(:weather_who).id,
          _destroy: true
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          intent_id: intents(:weather_who).id
        }
      ]
    })
    assert interpretation.save
  end


  test 'Aliasname must be a valid Javascript variable' do
    interpretation_alias = interpretation_aliases(:weather_greeting_hello_who)

    interpretation_alias.aliasname = '7up'
    assert !interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'a-b'
    assert !interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'ab('
    assert !interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'function'
    assert !interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'o_O'
    assert interpretation_alias.validate

    interpretation_alias.aliasname = 'aBBa'
    assert interpretation_alias.validate

    interpretation_alias.aliasname = 'ab2'
    assert interpretation_alias.validate
  end
end
