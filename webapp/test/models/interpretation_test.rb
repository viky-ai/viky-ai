require 'test_helper'
require 'model_test_helper'

class InterpretationTest < ActiveSupport::TestCase

  test 'Basic interpretation creation & intent association' do
    interpretation = Interpretation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    interpretation.intent = intents(:weather_forecast)
    assert interpretation.save
    interpretation_alias = InterpretationAlias.new(
      aliasname: 'who',
      position_start: 0,
      position_end: 12
    )
    interpretation_alias.interpretation = interpretation
    interpretation_alias.interpretation_aliasable = intents(:weather_question)
    assert interpretation_alias.save

    assert_equal 2, interpretation.position
    assert_equal 'Good morning John', interpretation.expression
    assert_equal intents(:weather_forecast).id, interpretation.intent.id
    assert_equal false, interpretation.keep_order
    assert_equal 'close', interpretation.proximity.to_s
    assert interpretation.solution.nil?
    assert_equal 3, intents(:weather_forecast).interpretations.count
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
    interpretation.intent = intents(:weather_forecast)
    assert !interpretation.save

    expected = {
      locale: ['is not included in the list']
    }
    assert_equal expected, interpretation.errors.messages
  end


  test 'Interpretation destroy' do
    interpretation = interpretations(:weather_forecast_tomorrow)
    interpretation_id = interpretation.id

    assert_equal 1, Interpretation.where(id: interpretation_id).count
    assert_equal 2, interpretation.interpretation_aliases.count
    assert interpretation.destroy
    assert_equal 0, Interpretation.where(id: interpretation_id).count
    assert_equal 0, interpretation.interpretation_aliases.count
  end


  test 'Filter interpretations by locale' do
    intent = intents(:weather_forecast)
    en_interpretations = intent.interpretations_with_local('en')
    assert_equal 1, en_interpretations.count
    fr_interpretations = intent.interpretations_with_local('fr')
    assert_equal 1, fr_interpretations.count
  end


  test 'Check interpretation solution is valid' do
    interpretation = interpretations(:weather_forecast_tomorrow)
    interpretation.solution = (['a'] * 5001).join('')
    assert !interpretation.valid?
    expected = {
      solution: ['is too long (maximum is 5000 characters)']
    }
    assert_equal expected, interpretation.errors.messages
  end


  test 'Update interpretations positions' do
    intent = intents(:weather_forecast)
    assert intent.interpretations.destroy_all

    interpretation_0 = Interpretation.create(
      expression: 'interpretation_0',
      locale: Locales::ANY,
      position: 0,
      intent: intent
    )
    interpretation_1 = Interpretation.create(
      expression: 'interpretation_1',
      locale: Locales::ANY,
      position: 1,
      intent: intent
    )
    interpretation_2 = Interpretation.create(
      expression: 'interpretation_2',
      locale: Locales::ANY,
      position: 2,
      intent: intent
    )

    new_positions = [interpretation_1.id, interpretation_2.id, interpretation_0.id, '132465789']
    Interpretation.update_positions(intent, new_positions)

    force_reset_model_cache([interpretation_0, interpretation_1, interpretation_2])
    assert_equal [2, 1, 0], [interpretation_1.position, interpretation_2.position, interpretation_0.position]
  end


  test 'Sanitize HTML expression' do
    interpretation = interpretations(:weather_forecast_tomorrow)
    interpretation.expression = '<div><strong>themselves</strong></div>'
    assert interpretation.save
    assert_equal 'themselves', interpretation.expression

    interpretation.expression = '<script>javascript:alert("Hello")</script>'
    assert interpretation.save
    assert_equal '', interpretation.expression
  end
end
