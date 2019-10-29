require 'test_helper'

class FormulationAliasTest < ActiveSupport::TestCase

  test 'belongs to validation on different formulation alias nature' do
    number = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'number',
      formulation_id: formulations(:weather_forecast_demain).id,
      nature: 'type_number'
    )
    assert number.save

    type_intent = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who',
      formulation_id: formulations(:weather_forecast_demain).id,
      nature: 'type_intent'
    )
    assert_not type_intent.save
    assert_equal ["Intent can't be blank"], type_intent.errors.full_messages

    type_entities_list = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who',
      formulation_id: formulations(:weather_forecast_demain).id,
      nature: 'type_entities_list'
    )
    assert_not type_entities_list.save
    assert_equal ["Entities list can't be blank"], type_entities_list.errors.full_messages
  end


  test 'remove intent used as alias remove related formulation alias' do
    weather_question_intent = intents(:weather_question)
    formulation = formulations(:weather_forecast_tomorrow)

    assert_equal weather_question_intent.id, formulation.formulation_aliases.first.formulation_aliasable.id
    assert_equal 2, formulation.formulation_aliases.count

    assert weather_question_intent.destroy
    assert_equal 1, formulation.formulation_aliases.count
  end


  test 'aliases no overlap' do
    # Create with 1 range
    formulation = Formulation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      formulation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert formulation.save

    # Create with 2 ranges without overlap
    formulation = Formulation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      formulation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who1',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who2',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert formulation.save

    # Create with 2 ranges with overlap
    formulation = Formulation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      formulation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_not formulation.save
    assert_equal ["Formulation aliases position overlap"], formulation.errors.full_messages

    # Update with 2 ranges with overlap
    assert_not formulation.update({
      formulation_aliases_attributes: [
        {
          position_start: 8,
          position_end: 21,
          aliasname: 'who1',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 5,
          position_end: 14,
          aliasname: 'who2',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_equal ["Formulation aliases position overlap"], formulation.errors.full_messages


    # Update with 3 ranges
    assert_not formulation.update({
      formulation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 5,
          aliasname: 'who3',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 6,
          position_end: 10,
          aliasname: 'who4',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 8,
          position_end: 12,
          aliasname: 'who5',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_equal ["Formulation aliases position overlap"], formulation.errors.full_messages
  end


  test 'Basic alias creation & formulation association' do
    formulation_alias = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    weather_forecast_demain = formulations(:weather_forecast_demain)
    formulation_alias.formulation = weather_forecast_demain

    weather_who = intents(:weather_question)
    formulation_alias.formulation_aliasable = weather_who

    assert formulation_alias.save
    assert_equal "type_intent", formulation_alias.nature
    assert formulation_alias.type_intent?
    assert_not formulation_alias.type_number?
    assert_equal 8, formulation_alias.position_start
    assert_equal 21, formulation_alias.position_end
    assert_equal 'who', formulation_alias.aliasname
    assert_not formulation_alias.is_list
    assert_equal weather_forecast_demain.id, formulation_alias.formulation.id
    assert_equal weather_who.id, formulation_alias.formulation_aliasable.id
  end


  test 'Aliasname must be present' do
    formulation_alias = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      formulation: formulations(:weather_forecast_demain),
      formulation_aliasable: intents(:weather_question)
    )
    assert_not formulation_alias.save
    expected = ['Parameter name can\'t be blank', 'Parameter name is invalid']
    assert_equal expected, formulation_alias.errors.full_messages
  end


  test 'Error on start position after end position' do
    formulation_alias = FormulationAlias.new(
      aliasname: 'who',
      position_start: 2,
      position_end: 1
    )
    formulation_alias.formulation = formulations(:weather_forecast_demain)
    formulation_alias.formulation_aliasable = intents(:weather_question)

    assert_not formulation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, formulation_alias.errors.full_messages
  end


  test 'Error on negative start and end positions' do
    formulation_alias = FormulationAlias.new(
      aliasname: 'who',
      position_start: -9,
      position_end: -3
    )
    formulation_alias.formulation = formulations(:weather_forecast_demain)
    formulation_alias.formulation_aliasable = intents(:weather_question)

    assert_not formulation_alias.validate
    expected = ['Position start must be greater than or equal to 0', 'Position end must be greater than 0']
    assert_equal expected, formulation_alias.errors.full_messages
  end


  test 'Error start and end positions are egals' do
    formulation_alias = FormulationAlias.new(
      aliasname: 'who',
      position_start: 4,
      position_end: 4
    )
    formulation_alias.formulation = formulations(:weather_forecast_demain)
    formulation_alias.formulation_aliasable = intents(:weather_question)

    assert_not formulation_alias.validate
    expected = ['Position end must be greater than position start']
    assert_equal expected, formulation_alias.errors.full_messages
  end


  test 'Check aliasname uniqueness' do
    formulation = Formulation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      formulation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 11,
          aliasname: 'who',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert_not formulation.save
    expected = ['Formulation aliases aliasname has already been taken']
    assert_equal expected, formulation.errors.full_messages
  end


  test 'Redefine deleted aliasname' do
    formulation = formulations(:weather_forecast_tomorrow)

    expected = ["question", "when"]
    assert_equal expected, formulation.formulation_aliases.pluck(:aliasname)

    assert formulation.update({
      formulation_aliases_attributes: [
        {
          id: formulation_aliases(:weather_forecast_tomorrow_question).id,
          position_start: 6,
          position_end: 11,
          aliasname: 'who',
          formulation_aliasable: intents(:weather_question),
          _destroy: true
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    expected = ["when", "who"]
    assert_equal expected, formulation.formulation_aliases.pluck(:aliasname)
  end


  test 'Aliasname must be a valid Javascript variable' do
    formulation_alias = formulation_aliases(:weather_forecast_tomorrow_question)

    formulation_alias.aliasname = '7up'
    assert_not formulation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, formulation_alias.errors.full_messages

    formulation_alias.aliasname = 'a-b'
    assert_not formulation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, formulation_alias.errors.full_messages

    formulation_alias.aliasname = 'ab('
    assert_not formulation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, formulation_alias.errors.full_messages

    formulation_alias.aliasname = 'function'
    assert_not formulation_alias.validate
    expected = ['Parameter name is invalid']
    assert_equal expected, formulation_alias.errors.full_messages

    formulation_alias.aliasname = 'o_O'
    assert formulation_alias.validate

    formulation_alias.aliasname = 'aBBa'
    assert formulation_alias.validate

    formulation_alias.aliasname = 'ab2'
    assert formulation_alias.validate
  end


  test 'Aliasname can be of maximum 2048 bytes' do
    formulation_alias = formulation_aliases(:weather_forecast_tomorrow_question)

    formulation_alias.aliasname = 'Ò' * 1025
    assert_not formulation_alias.validate
    assert_equal 'Parameter name (2.002 KB) is too long (maximum is 2 KB)', formulation_alias.errors.full_messages[0]
  end


  test 'Invert alias names' do
    formulation = Formulation.new({
      expression: 'test',
      intent_id: intents(:weather_forecast).id,
      locale: 'fr',
      formulation_aliases_attributes: [
        {
          position_start: 0,
          position_end: 11,
          aliasname: 'who1',
          formulation_aliasable: intents(:weather_question)
        },
        {
          position_start: 15,
          position_end: 21,
          aliasname: 'who2',
          formulation_aliasable: intents(:weather_question)
        }
      ]
    })
    assert formulation.save
    expected = ["who1", "who2"]
    assert_equal expected, formulation.formulation_aliases.pluck(:aliasname)

    assert formulation.update({
       formulation_aliases_attributes: [
         {
           id: formulation.formulation_aliases.first.id,
           position_start: 0,
           position_end: 11,
           aliasname: formulation.formulation_aliases.last.aliasname,
           formulation_aliasable: intents(:weather_question)
         },
         {
           id: formulation.formulation_aliases.last.id,
           position_start: 15,
           position_end: 21,
           aliasname: formulation.formulation_aliases.first.aliasname,
           formulation_aliasable: intents(:weather_question)
         }
       ]
    })
    expected = ["who2", "who1"]
    assert_equal expected, formulation.formulation_aliases.pluck(:aliasname)
  end


  test 'Alias type regex should have a regular expression' do
    regexp_invalid = FormulationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      formulation_id: formulations(:terminator_find_sarah).id,
      nature: 'type_regex'
    )
    assert_not regexp_invalid.save
    assert_not_nil regexp_invalid.errors[:reg_exp]
    expected_msg = ['Regular expression can\'t be blank']
    assert_equal expected_msg, regexp_invalid.errors.full_messages

    regexp_valid = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'name',
      formulation_id: formulations(:terminator_find_sarah).id,
      nature: 'type_regex',
      reg_exp: '[a-b]'
    )
    assert regexp_valid.save
  end


  test 'Regular expression should be valid' do
    regexp_invalid = FormulationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      formulation_id: formulations(:terminator_find_sarah).id,
      nature: 'type_regex',
      reg_exp: '['
    )
    assert_not regexp_invalid.save
    assert_not_nil regexp_invalid.errors[:reg_exp]
    expected_msg = ['Regular expression should be valid']
    assert_equal expected_msg, regexp_invalid.errors.full_messages
  end


  test 'Regular expression should be less than 4096 bytes' do
    test_regexp = FormulationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'where',
      formulation_id: formulations(:terminator_find_sarah).id,
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
    formulation_alias = FormulationAlias.new(
      aliasname: "say_my_name",
      position_start: 8,
      position_end: 21,
      formulation: formulations(:weather_forecast_demain),
      formulation_aliasable: intents(:weather_question),
      is_list: true,
      any_enabled: true,
    )
    assert_not formulation_alias.save
    expected = ['Options "List" and "Any" are not compatibles on the same annotation.']
    assert_equal expected, formulation_alias.errors.full_messages

    formulation_alias.any_enabled = false
    assert formulation_alias.save
  end
end
