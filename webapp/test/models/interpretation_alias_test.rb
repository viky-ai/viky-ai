require 'test_helper'

class InterpretationAliasTest < ActiveSupport::TestCase

  test 'belongs to validation on different interpretation alias nature' do
    number = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'number',
      interpretation_id: interpretations(:weather_forecast_demain).id,
      nature: 'type_number'
    )
    assert number.save

    type_intent = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who',
      interpretation_id: interpretations(:weather_forecast_demain).id,
      nature: 'type_intent'
    )
    assert_not type_intent.save
    assert_equal ["Intent can't be blank"], type_intent.errors.full_messages

    type_entities_list = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who',
      interpretation_id: interpretations(:weather_forecast_demain).id,
      nature: 'type_entities_list'
    )
    assert_not type_entities_list.save
    assert_equal ["Entities list can't be blank"], type_entities_list.errors.full_messages
  end


  test 'remove intent used as alias remove related interpretation alias' do
    weather_question_intent = intents(:weather_question)
    interpretation = interpretations(:weather_forecast_tomorrow)

    assert_equal weather_question_intent.id, interpretation.interpretation_aliases.first.interpretation_aliasable.id
    assert_equal 2, interpretation.interpretation_aliases.count

    assert weather_question_intent.destroy
    assert_equal 1, interpretation.interpretation_aliases.count
  end


  test 'aliases no overlap' do
    # Create with 1 range
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert interpretation.save

    # Create with 2 ranges without overlap
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who1',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who2',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert interpretation.save

    # Create with 2 ranges with overlap
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      interpretation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_not interpretation.save
    assert_equal ["Interpretation aliases position overlap"], interpretation.errors.full_messages

    # Update with 2 ranges with overlap
    assert_not interpretation.update({
      interpretation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_equal ["Interpretation aliases position overlap"], interpretation.errors.full_messages


    # Update with 3 ranges
    assert_not interpretation.update({
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who3',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who4',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 8,
          position_end: 12,
          aliasname: 'who5',
          interpretation_aliasable: intents(:weather_question)
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
    weather_forecast_demain = interpretations(:weather_forecast_demain)
    interpretation_alias.interpretation = weather_forecast_demain

    weather_who = intents(:weather_question)
    interpretation_alias.interpretation_aliasable = weather_who

    assert interpretation_alias.save
    assert_equal "type_intent", interpretation_alias.nature
    assert interpretation_alias.type_intent?
    assert_not interpretation_alias.type_number?
    assert_equal 8, interpretation_alias.position_start
    assert_equal 21, interpretation_alias.position_end
    assert_equal 'who', interpretation_alias.aliasname
    assert_not interpretation_alias.is_list
    assert_equal weather_forecast_demain.id, interpretation_alias.interpretation.id
    assert_equal weather_who.id, interpretation_alias.interpretation_aliasable.id
  end


  test 'Aliasname must be present' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      interpretation: interpretations(:weather_forecast_demain),
      interpretation_aliasable: intents(:weather_question)
    )
    assert_not interpretation_alias.save
    expected = ['Parameter name can\'t be blank', 'Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error on start position after end position' do
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
      position_start: 2,
      position_end: 1
    )
    interpretation_alias.interpretation = interpretations(:weather_forecast_demain)
    interpretation_alias.interpretation_aliasable = intents(:weather_question)

    assert_not interpretation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error on negative start and end positions' do
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
      position_start: -9,
      position_end: -3
    )
    interpretation_alias.interpretation = interpretations(:weather_forecast_demain)
    interpretation_alias.interpretation_aliasable = intents(:weather_question)

    assert_not interpretation_alias.validate
    expected = ['Position start must be greater than or equal to 0', 'Position end must be greater than 0']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Error start and end positions are egals' do
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
      position_start: 4,
      position_end: 4
    )
    interpretation_alias.interpretation = interpretations(:weather_forecast_demain)
    interpretation_alias.interpretation_aliasable = intents(:weather_question)

    assert_not interpretation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, interpretation_alias.errors.full_messages
  end


  test 'Check aliasname uniqueness' do
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 11,
          aliasname: 'who',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_not interpretation.save
    expected = ['Interpretation aliases aliasname has already been taken']
    assert_equal expected, interpretation.errors.full_messages
  end


  test 'Redefine deleted aliasname' do
    interpretation = interpretations(:weather_forecast_tomorrow)

    expected = ["question", "when"]
    assert_equal expected, interpretation.interpretation_aliases.pluck(:aliasname)

    assert interpretation.update({
      interpretation_aliases_attributes: [
        {
          id: interpretation_aliases(:weather_forecast_tomorrow_question).id,
          position_start: 6,
          position_end: 11,
          aliasname: 'who',
          interpretation_aliasable: intents(:weather_question),
          _destroy: true
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    expected = ["when", "who"]
    assert_equal expected, interpretation.interpretation_aliases.pluck(:aliasname)
  end


  test 'Aliasname must be a valid Javascript variable' do
    interpretation_alias = interpretation_aliases(:weather_forecast_tomorrow_question)

    interpretation_alias.aliasname = '7up'
    assert_not interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'a-b'
    assert_not interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'ab('
    assert_not interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'function'
    assert_not interpretation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.aliasname = 'o_O'
    assert interpretation_alias.validate

    interpretation_alias.aliasname = 'aBBa'
    assert interpretation_alias.validate

    interpretation_alias.aliasname = 'ab2'
    assert interpretation_alias.validate
  end


  test 'Aliasname can be of maximum 2048 bytes' do
    interpretation_alias = interpretation_aliases(:weather_forecast_tomorrow_question)

    interpretation_alias.aliasname = 'Ò' * 1025
    assert_not interpretation_alias.validate
    assert_equal 'Parameter name (2.002 KB) is too long (maximum is 2 KB)', interpretation_alias.errors.full_messages[0]
  end


  test 'Invert alias names' do
    interpretation = Interpretation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      interpretation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 11,
          aliasname: 'who1',
          interpretation_aliasable: intents(:weather_question)
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who2',
          interpretation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert interpretation.save
    expected = ["who1", "who2"]
    assert_equal expected, interpretation.interpretation_aliases.pluck(:aliasname)

    assert interpretation.update({
       interpretation_aliases_attributes: [
         {
           id: interpretation.interpretation_aliases.first.id,
           position_start: 0,
           position_end: 11,
           aliasname: interpretation.interpretation_aliases.last.aliasname,
           interpretation_aliasable: intents(:weather_question)
         },
         {
           id: interpretation.interpretation_aliases.last.id,
           position_start: 15,
           position_end: 21,
           aliasname: interpretation.interpretation_aliases.first.aliasname,
           interpretation_aliasable: intents(:weather_question)
         }
       ]
    })
    expected = ["who2", "who1"]
    assert_equal expected, interpretation.interpretation_aliases.pluck(:aliasname)
  end


  test 'Alias type regex should have a regular expression' do
    regexp_invalid = InterpretationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      interpretation_id: interpretations(:terminator_find_sarah).id,
      nature: 'type_regex'
    )
    assert_not regexp_invalid.save
    assert_not_nil regexp_invalid.errors[:reg_exp]
    expected_msg = ['Regular expression can\'t be blank']
    assert_equal expected_msg, regexp_invalid.errors.full_messages

    regexp_valid = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'name',
      interpretation_id: interpretations(:terminator_find_sarah).id,
      nature: 'type_regex',
      reg_exp: '[a-b]'
    )
    assert regexp_valid.save
  end


  test 'Regular expression should be valid' do
    regexp_invalid = InterpretationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      interpretation_id: interpretations(:terminator_find_sarah).id,
      nature: 'type_regex',
      reg_exp: '['
    )
    assert_not regexp_invalid.save
    assert_not_nil regexp_invalid.errors[:reg_exp]
    expected_msg = ['Regular expression should be valid']
    assert_equal expected_msg, regexp_invalid.errors.full_messages
  end


  test 'Regular expression should be less than 4096 bytes' do
    test_regexp = InterpretationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      interpretation_id: interpretations(:terminator_find_sarah).id,
      nature: 'type_regex',
      reg_exp: "[a#{'b' * 4097}c]"
    )
    assert_not test_regexp.save
    assert_not_nil test_regexp.errors[:reg_exp]
    assert_equal ['Regular expression (4.005 KB) is too long (maximum is 4 KB)'], test_regexp.errors.full_messages

    test_regexp.reg_exp = "[a#{'b' * 4000}c]"
    assert test_regexp.save
  end


  test 'Any and List option compability' do
    interpretation_alias = InterpretationAlias.new(
      aliasname: "say_my_name",
      position_start: 8,
      position_end: 21,
      interpretation: interpretations(:weather_forecast_demain),
      interpretation_aliasable: intents(:weather_question),
      is_list: true,
      any_enabled: true,
    )
    assert_not interpretation_alias.save
    expected = ['Options "List" and "Any" are not compatibles on the same annotation.']
    assert_equal expected, interpretation_alias.errors.full_messages

    interpretation_alias.any_enabled = false
    assert interpretation_alias.save
  end
end
