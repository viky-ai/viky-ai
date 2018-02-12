require 'test_helper'

class InterpretationTest < ActiveSupport::TestCase

  test 'Basic interpretation creation & intent association' do
    interpretation = Interpretation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    interpretation.intent = intents(:weather_greeting)
    assert interpretation.save
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
      position_start: 0,
      position_end: 12
    )
    interpretation_alias.interpretation = interpretation
    interpretation_alias.interpretation_aliasable = intents(:weather_who)
    assert interpretation_alias.save

    assert_equal 1, interpretation.position
    assert_equal 'Good morning John', interpretation.expression
    assert_equal intents(:weather_greeting).id, interpretation.intent.id
    assert_equal false, interpretation.keep_order
    assert_equal false, interpretation.glued
    assert interpretation.solution.nil?
    assert_equal 3, intents(:weather_greeting).interpretations.count
    assert_equal interpretation_alias.id, interpretation.interpretation_aliases.reload[0].id
  end


  test 'Expression and locale can\'t be blank and must be linked to an intent' do
    interpretation = Interpretation.new(expression: '')
    assert !interpretation.save

    expected = {
      intent: ['must exist'],
      expression: ['can\'t be blank'],
      locale: ['is not included in the list', 'can\'t be blank']
    }
    assert_equal expected, interpretation.errors.messages
  end


  test 'Locale must be in list' do
    interpretation = Interpretation.new(expression: 'Good morning', locale: 'toto')
    interpretation.intent = intents(:weather_greeting)
    assert !interpretation.save

    expected = {
      locale: ['is not included in the list']
    }
    assert_equal expected, interpretation.errors.messages
  end


  test 'Interpretation destroy' do
    interpretation = interpretations(:weather_greeting_hello)
    interpretation_id = interpretation.id

    assert_equal 1, Interpretation.where(id: interpretation_id).count
    assert_equal 1, interpretation.interpretation_aliases.count
    assert interpretation.destroy
    assert_equal 0, Interpretation.where(id: interpretation_id).count
    assert_equal 0, interpretation.interpretation_aliases.count
  end


  test 'Filter interpretations by locale' do
    intent = intents(:weather_greeting)
    en_interpretations = intent.interpretations_with_local('en')
    assert_equal 1, en_interpretations.count
    fr_interpretations = intent.interpretations_with_local('fr')
    assert_equal 1, fr_interpretations.count
  end


  test 'Check interpretation solution is valid' do
    interpretation = interpretations(:weather_greeting_hello)
    interpretation.solution = (['a'] * 2001).join('')
    assert !interpretation.valid?
    expected = {
      solution: ['is too long (maximum is 2000 characters)']
    }
    assert_equal expected, interpretation.errors.messages
  end
end
