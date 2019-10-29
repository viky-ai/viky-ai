require 'test_helper'
require 'model_test_helper'

class FormulationTest < ActiveSupport::TestCase

  test 'Basic formulation creation & intent association' do
    formulation = Formulation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    formulation.intent = intents(:weather_forecast)
    assert formulation.save
    formulation_alias = FormulationAlias.new(
      aliasname: 'who',
      position_start: 0,
      position_end: 12
    )
    formulation_alias.formulation = formulation
    formulation_alias.formulation_aliasable = intents(:weather_question)
    assert formulation_alias.save

    assert_equal 1, formulation.position
    assert_equal 'Good morning John', formulation.expression
    assert_equal intents(:weather_forecast).id, formulation.intent.id
    assert_not formulation.keep_order
    assert formulation.proximity_close?
    assert formulation.solution.nil?
    assert_equal 3, intents(:weather_forecast).formulations.count
    assert_equal formulation_alias.id, formulation.formulation_aliases.reload[0].id
  end


  test 'Expression and locale can\'t be blank and must be linked to an intent' do
    formulation = Formulation.new(expression: '')
    assert_not formulation.save

    expected = {
      intent: ['must exist'],
      expression: ['can\'t be blank'],
      locale: ['is not included in the list', 'can\'t be blank']
    }
    assert_equal expected, formulation.errors.messages
  end


  test 'Locale must be in list' do
    formulation = Formulation.new(expression: 'Good morning', locale: 'toto')
    formulation.intent = intents(:weather_forecast)
    assert_not formulation.save

    expected = {
      locale: ['is not included in the list']
    }
    assert_equal expected, formulation.errors.messages
  end


  test 'Formulation destroy' do
    formulation = formulations(:weather_forecast_tomorrow)
    formulation_id = formulation.id

    assert_equal 1, Formulation.where(id: formulation_id).count
    assert_equal 2, formulation.formulation_aliases.count
    assert formulation.destroy
    assert_equal 0, Formulation.where(id: formulation_id).count
    assert_equal 0, formulation.formulation_aliases.count
  end


  test 'Filter formulations by locale' do
    intent = intents(:weather_forecast)
    en_formulations = intent.formulations_with_local('en')
    assert_equal 1, en_formulations.count
    fr_formulations = intent.formulations_with_local('fr')
    assert_equal 1, fr_formulations.count
  end


  test 'Check formulation solution is valid' do
    formulation = formulations(:weather_forecast_tomorrow)
    formulation.solution = (['a'] * 8193).join('')
    assert_not formulation.valid?
    expected = {
      solution: ['(8.001 KB) is too long (maximum is 8 KB)']
    }
    assert_equal expected, formulation.errors.messages
  end


  test 'Check expression size' do
    formulation = formulations(:weather_forecast_tomorrow)
    formulation.expression = (['À'] * 1025).join('')
    assert_not formulation.valid?
    expected = {
      expression: ['(2.002 KB) is too long (maximum is 2 KB)']
    }
    assert_equal expected, formulation.errors.messages
  end


  test "Aliases any constraint : only one any" do
    formulation = formulations(:weather_forecast_demain)

    assert_not formulation.update(
      formulation_aliases_attributes: [
        {
          aliasname: "first",
          position_start: 0,
          position_end: 4,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          any_enabled: true,
        },
        {
          aliasname: "second",
          position_start: 6,
          position_end: 10,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          any_enabled: true,
        }
      ]
    )
    expected = ["Only one annotation with \"Any\" option is permitted."]
    assert_equal expected, formulation.errors.full_messages
  end


  test "Aliases any constraint : one alias one any" do
    formulation = formulations(:weather_forecast_demain)
    assert_not formulation.update(
      formulation_aliases_attributes: [
        {
          aliasname: "first",
          position_start: 0,
          position_end: 4,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          any_enabled: true,
        }
      ]
    )
    expected = ["\"Any\" option is not permitted with only one annotation."]
    assert_equal expected, formulation.errors.full_messages


    formulation.reload
    assert formulation.update(
      formulation_aliases_attributes: [
        {
          aliasname: "first",
          position_start: 0,
          position_end: 4,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          any_enabled: true,
        },
        {
          aliasname: "second",
          position_start: 5,
          position_end: 10,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          any_enabled: false,
        }
      ]
    )

    assert_not formulation.update(
      formulation_aliases_attributes: [
        {
          id: formulation.formulation_aliases.last.id,
          _destroy:  true,
        }
      ]
    )
    expected = ["\"Any\" option is not permitted with only one annotation."]
    assert_equal expected, formulation.errors.full_messages
  end


  test "Aliases any constraint : only one list" do
    formulation = formulations(:weather_forecast_demain)

    assert_not formulation.update(
      formulation_aliases_attributes: [
        {
          aliasname: "first",
          position_start: 0,
          position_end: 4,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          is_list: true,
        },
        {
          aliasname: "second",
          position_start: 6,
          position_end: 10,
          formulation: formulations(:weather_forecast_demain),
          formulation_aliasable: intents(:weather_question),
          is_list: true,
        }
      ]
    )
    expected = ["Only one annotation with \"List\" option is permitted."]
    assert_equal expected, formulation.errors.full_messages
  end


  test 'Update formulations positions' do
    intent = intents(:weather_forecast)
    assert intent.formulations.destroy_all

    formulation_0 = Formulation.create(
      expression: 'formulation_0',
      locale: Locales::ANY,
      position: 0,
      intent: intent
    )
    formulation_1 = Formulation.create(
      expression: 'formulation_1',
      locale: Locales::ANY,
      position: 1,
      intent: intent
    )
    formulation_2 = Formulation.create(
      expression: 'formulation_2',
      locale: Locales::ANY,
      position: 2,
      intent: intent
    )

    new_positions = [formulation_1.id, formulation_2.id, formulation_0.id, '132465789']
    Formulation.update_positions(intent, new_positions)

    force_reset_model_cache([formulation_0, formulation_1, formulation_2])
    assert_equal [2, 1, 0], [formulation_1.position, formulation_2.position, formulation_0.position]
  end

  test 'Reorder formulations positions within the given set of positions only' do
    intent = intents(:weather_forecast)
    assert intent.formulations.destroy_all

    formulation_0 = Formulation.create(
      expression: 'formulation_0',
      locale: 'en',
      position: 0,
      intent: intent
    )
    formulation_1 = Formulation.create(
      expression: 'formulation_1',
      locale: 'fr',
      position: 1,
      intent: intent
    )
    formulation_2 = Formulation.create(
      expression: 'formulation_2',
      locale: 'en',
      position: 2,
      intent: intent
    )
    formulation_3 = Formulation.create(
      expression: 'formulation_2',
      locale: 'fr',
      position: 3,
      intent: intent
    )

    new_positions = [formulation_1.id, formulation_3.id]
    Formulation.update_positions(intent, new_positions)
    force_reset_model_cache([formulation_0, formulation_1, formulation_2, formulation_3])
    assert_equal [3, 2, 1, 0], [formulation_1.position, formulation_2.position, formulation_3.position, formulation_0.position]

    new_positions = [formulation_0.id, formulation_2.id]
    Formulation.update_positions(intent, new_positions)
    force_reset_model_cache([formulation_0, formulation_1, formulation_2, formulation_3])
    assert_equal [3, 2, 1, 0], [formulation_1.position, formulation_0.position, formulation_3.position, formulation_2.position]

  end


  test 'Sanitize HTML expression' do
    formulation = formulations(:weather_forecast_tomorrow)
    formulation.expression = '<div><strong>themselves</strong></div>'
    assert formulation.save
    assert_equal 'themselves', formulation.expression

    formulation.expression = '<script>javascript:alert("Hello")</script>'
    assert formulation.save
    assert_equal '', formulation.expression
  end


  test 'Limit maximum number of words in an formulation' do
    formulation = formulations(:weather_forecast_demain)
    formulation.expression = (['a'] * 37).join(' ')
    assert_not formulation.save
    expected = {
      expression: ['is too long (maximum is 36 elements), found: 37']
    }
    assert_equal expected, formulation.errors.messages

    formulation.expression = 'a1.2' * 15
    assert_not formulation.save
    expected = {
      expression: ['is too long (maximum is 36 elements), found: 60']
    }
    assert_equal expected, formulation.errors.messages

    formulation.expression = ([
      '²', # G_UNICODE_OTHER_NUMBER (No)
      '_', # G_UNICODE_CONNECT_PUNCTUATION (Pc)
      '-', # G_UNICODE_DASH_PUNCTUATION (Pd)
      '(', # G_UNICODE_OPEN_PUNCTUATION (Ps)
      ')', # G_UNICODE_CLOSE_PUNCTUATION (Pe)
      '«', # G_UNICODE_INITIAL_PUNCTUATION (Pi)
      '”', # G_UNICODE_FINAL_PUNCTUATION (Pf)
      '&', # G_UNICODE_OTHER_PUNCTUATION (Po)
      '€', # G_UNICODE_CURRENCY_SYMBOL (Sc)
      '^', # G_UNICODE_MODIFIER_SYMBOL (Sk)
      '=', # G_UNICODE_MATH_SYMBOL (Sm)
      '©', # G_UNICODE_OTHER_SYMBOL (So)
      '力', # G_UNICODE_BREAK_IDEOGRAPHIC (ID)
      '😀', # G_UNICODE_BREAK_EMOJI_BASE (EB)
      '🏿',  # G_UNICODE_BREAK_EMOJI_MODIFIER (EM)
    ] * 3).join

    assert_not formulation.save
    expected = {
      expression: ['is too long (maximum is 36 elements), found: 45']
    }
    assert_equal expected, formulation.errors.messages

    formulation.expression = (['a'] * 36 + ['؀' * 2]).join(' ')
    assert formulation.save

    formulation.expression = (['a'] * 35 + ['après demain']).join(' ')
    assert FormulationAlias.new(
      aliasname: 'when',
      position_start: 70,
      position_end: 82,
      formulation: formulation,
      formulation_aliasable: entities_lists(:weather_dates)
    ).save
    force_reset_model_cache formulation
    assert formulation.save
  end

  test 'Change formulation locale' do
    forecast = formulations(:weather_forecast_tomorrow)
    previous_locale = ''
    assert_difference "Formulation.where(locale: '*').count" do
      previous_locale = forecast.update_locale '*'
    end
    assert_equal 'en', previous_locale
  end

  test 'Position formulation in new locale' do
    intent = intents(:weather_forecast)
    assert intent.formulations.destroy_all

    Formulation.create(
      expression: 'formulation_0',
      locale: '*',
      position: 0,
      intent: intent
    )
    formulation_1 = Formulation.create(
      expression: 'formulation_1',
      locale: '*',
      position: 1,
      intent: intent
    )

    previous_locale = ''
    assert_difference "Formulation.where(locale: 'en').count" do
      previous_locale = formulation_1.update_locale 'en'
    end
    assert_equal '*', previous_locale
  end

  test 'Formulation creation scope position to language' do
    intent = intents(:weather_forecast)
    assert intent.formulations.destroy_all

    Formulation.create(
      expression: 'formulation_0',
      locale: '*',
      intent: intent
    )
    Formulation.create(
      expression: 'formulation_1',
      locale: '*',
      intent: intent
    )
    english_formulation = Formulation.create(
      expression: 'formulation_3',
      locale: 'en',
      intent: intent
    )
    assert_equal 0, english_formulation.position
  end

  test 'Save formulation with very locales' do
    intent = intents(:weather_forecast)
    Locales::ALL.each do |locale|
      formulation = Formulation.new(
        expression: "formulation_#{locale}",
        locale: locale,
        intent: intent
      )
      assert formulation.save
    end
  end

  test 'Quota for formulation and entities' do
    Feature.with_quota_enabled do
      Quota.stubs(:expressions_limit).returns(11)

      user = users(:admin)
      assert_equal 10, user.expressions_count

      formulation = Formulation.new(
        expression: 'Sunny tomorrow morning',
        locale: 'en'
      )
      formulation.intent = intents(:weather_forecast)
      assert formulation.save

      formulation_next = Formulation.new(
        expression: 'Rainy tomorrow afternoon',
        locale: 'en'
      )
      formulation_next.intent = intents(:weather_forecast)
      assert_not formulation_next.save
      expected = ['Quota exceeded (maximum is 11 formulations and entities)']
      assert_equal expected, formulation_next.errors.full_messages
    end
  end
end
