require 'test_helper'

class PackageTest < ActiveSupport::TestCase

  test "package generation" do
    weather = agents(:weather)
    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id"    => intents(:weather_forecast).id,
          "slug"  => "admin/weather/interpretations/weather_forecast",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "Quel temps fera-t-il demain ?",
              "locale"     => "fr",
              "solution"   => "Quel temps fera-t-il demain ?"
            },
            {
              "expression" => "@{question} @{when} ?",
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question",
                  "id"      => intents(:weather_question).id,
                  "package" => weather.id
                },
                {
                  "alias"   => "when",
                  "slug"    => "admin/weather/entities_lists/weather_dates",
                  "id"      => entities_lists(:weather_dates).id,
                  "package" => weather.id
                }
              ],
              "locale"     => "en",
              "keep-order" => true,
              "glued"      => true,
              "solution"   => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id"    => intents(:weather_question).id,
          "slug"  => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "What the weather like",
              "locale"     => "en",
              "solution"   => "What the weather like"
            }
          ]
        },
        {
          "id"       => entities_lists(:weather_conditions).id,
          "slug"     => "admin/weather/entities_lists/weather_conditions",
          'scope'    => 'public',
          "expressions" => [
            {
              "expression" => "soleil",
              "locale"     => "fr",
              "solution" => "`weather: sunny`"
            },
            {
              "expression" => "sun",
              "locale"     => "en",
              "solution" => "`weather: sunny`"
            },
            {
              "expression" => "pluie",
              "locale"     => "fr",
              "solution" => "`weather: raining`"
            },
            {
              "expression" => "rain",
              "locale"     => "en",
              "solution" => "`weather: raining`"
            }
          ]
        },
        {
          "id"       => entities_lists(:weather_dates).id,
          "slug"     => "admin/weather/entities_lists/weather_dates",
          'scope'    => 'public',
          "expressions" => [
            {
              "expression" => "aujourd'hui",
              "locale"     => "fr",
              "solution" => "`date: today`"
            },
            {
              "expression" => "tout à l'heure",
              "locale"     => "fr",
              "solution" => "`date: today`"
            },
            {
              "expression" => "today",
              "locale"     => "en",
              "solution" => "`date: today`"
            },
            {
              "expression" => "tomorrow",
              "solution" => "`date: tomorrow`"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Package generation with locale any' do
    weather = agents(:weather)
    intent = intents(:weather_forecast)
    interpretation = interpretations(:weather_forecast_demain)
    interpretation.locale = Locales::ANY
    interpretation.save
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    intent.interpretations = [interpretation]
    intent.save
    assert intents(:weather_question).destroy

    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id"    => intent.id,
          "slug"  => "admin/weather/interpretations/weather_forecast",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "Quel temps fera-t-il demain ?",
              "solution"   => "Quel temps fera-t-il demain ?"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Package generation with private intent' do
    weather = agents(:weather)
    intent = intents(:weather_question)
    intent.visibility = Intent.visibilities[:is_private]
    intent.save
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    assert intents(:weather_forecast).destroy

    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id"    => intents(:weather_question).id,
          "slug"  => "admin/weather/interpretations/weather_question",
          'scope' => 'private',
          "expressions" => [
            {
              "expression" => "What the weather like",
              "locale"     => "en",
              "solution"   => "What the weather like"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Package generation with alias list' do
    weather = agents(:weather)
    ialias = interpretation_aliases(:weather_forecast_tomorrow_question)
    ialias.is_list = true
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    assert ialias.save

    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id" => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
          "slug" => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
          'scope' => 'hidden',
          "expressions" => [
            {
              "expression" => "@{question}",
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question",
                  "id"      => intents(:weather_question).id,
                  "package" => weather.id
                }
              ]
            },
            {
              "expression" => "@{question} @{question_recursive}",
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question",
                  "id"      => intents(:weather_question).id,
                  "package" => weather.id
                },
                {
                  "alias"   => "question_recursive",
                  "slug"    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  "id"      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  "package" => weather.id
                }
              ]
            }
          ]
        },
        {
          "id"    => intents(:weather_forecast).id,
          "slug"  => "admin/weather/interpretations/weather_forecast",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "Quel temps fera-t-il demain ?",
              "locale"     => "fr",
              "solution"   => "Quel temps fera-t-il demain ?"
            },
            {
              "expression" => "@{question} tomorrow ?",
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  "id"      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  "package" => weather.id
                }
              ],
              "locale"     => "en",
              "keep-order" => true,
              "glued"      => true,
              "solution"   => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id"    => intents(:weather_question).id,
          "slug"  => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "What the weather like",
              "locale"     => "en",
              "solution"   => "What the weather like"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Package generation with alias list any' do
    weather = agents(:weather)
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    ialias = interpretation_aliases(:weather_forecast_tomorrow_question)
    ialias.is_list = true
    ialias.any_enabled = true
    assert ialias.save

    p = Nlp::Package.new(weather)

    expected = {
      'id'   => weather.id,
      'slug' => 'admin/weather',
      'interpretations' => [
        {
          'id' => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
          'slug' => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
          'scope' => 'hidden',
          'expressions' => [
            {
              'expression' => '@{question}',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => 'admin/weather/interpretations/weather_question',
                  'id'      => intents(:weather_question).id,
                  'package' => weather.id
                }
              ]
            },
            {
              'expression' => '@{question} @{question_recursive}',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => 'admin/weather/interpretations/weather_question',
                  'id'      => intents(:weather_question).id,
                  'package' => weather.id
                },
                {
                  'alias'   => 'question_recursive',
                  'slug'    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  'id'      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  'package' => weather.id
                }
              ]
            },
            {
              'expression' => '@{question} @{question_recursive}',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'type'    => 'any'
                },
                {
                  'alias'   => 'question_recursive',
                  'slug'    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  'id'      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  'package' => weather.id
                }
              ]
            }
          ]
        },
        {
          'id'    => intents(:weather_forecast).id,
          'slug'  => 'admin/weather/interpretations/weather_forecast',
          'scope' => 'public',
          'expressions' => [
            {
              'expression' => 'Quel temps fera-t-il demain ?',
              'locale'     => 'fr',
              'solution'   => "Quel temps fera-t-il demain ?"
            },
            {
              'expression' => '@{question} tomorrow ?',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  'id'      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  'package' => weather.id
                }
              ],
              'locale'     => 'en',
              'keep-order' => true,
              'glued'      => true,
              'solution'   => '`forecast.tomorrow`'
            }
          ]
        },
        {
          'id'    => intents(:weather_question).id,
          'slug'  => 'admin/weather/interpretations/weather_question',
          'scope' => 'public',
          'expressions' => [
            {
              'expression' => 'What the weather like',
              'locale'     => 'en',
              'solution'   => "What the weather like"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Package generation with alias any' do
    weather = agents(:weather)
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    ialias = interpretation_aliases(:weather_forecast_tomorrow_question)
    ialias.any_enabled = true
    assert ialias.save

    p = Nlp::Package.new(weather)

    expected = {
      'id'   => weather.id,
      'slug' => 'admin/weather',
      'interpretations' => [
        {
          'id'    => intents(:weather_forecast).id,
          'slug'  => 'admin/weather/interpretations/weather_forecast',
          'scope' => 'public',
          'expressions' => [
            {
              'expression' => 'Quel temps fera-t-il demain ?',
              'locale'     => 'fr',
              'solution'   => 'Quel temps fera-t-il demain ?'
            },
            {
              'expression' => '@{question} tomorrow ?',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => 'admin/weather/interpretations/weather_question',
                  'id'      => intents(:weather_question).id,
                  'package' => weather.id
                }
              ],
              'locale'     => 'en',
              'keep-order' => true,
              'glued'      => true,
              'solution'   => '`forecast.tomorrow`'
            },
            {
              'expression' => '@{question} tomorrow ?',
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'type'    => 'any'
                }
              ],
              'locale'     => 'en',
              'keep-order' => true,
              'glued'      => true,
              'solution'   => '`forecast.tomorrow`'
            }
          ]
        },
        {
          'id'    => intents(:weather_question).id,
          'slug'  => 'admin/weather/interpretations/weather_question',
          'scope' => 'public',
          'expressions' => [
            {
              'expression' => 'What the weather like',
              'locale'     => 'en',
              'solution'   => "What the weather like"
            }
          ]
        }
      ]
    }
    assert_equal expected, p.generate_json
  end


  test 'Packages with all its dependencies' do
    weather = agents(:weather)
    terminator = agents(:terminator)
    assert AgentArc.create(source: weather, target: terminator)
    assert entities_lists(:weather_conditions).destroy
    assert entities_lists(:weather_dates).destroy
    weather.reload
    p = Nlp::Package.new(weather)

    expected = [{
      "id" => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id" => intents(:weather_forecast).id,
          "slug" => "admin/weather/interpretations/weather_forecast",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "Quel temps fera-t-il demain ?",
              "locale" => "fr",
              "solution" => "Quel temps fera-t-il demain ?"
            },
            {
              "expression" => "@{question} tomorrow ?",
              "aliases" => [
                {
                  "alias" => "question",
                  "slug" => "admin/weather/interpretations/weather_question",
                  "id" => intents(:weather_question).id,
                  "package" => weather.id
                }
              ],
              "locale" => "en",
              "keep-order" => true,
              "glued" => true,
              "solution" => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id" => intents(:weather_question).id,
          "slug" => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "What the weather like",
              "locale" => "en",
              "solution" => "What the weather like"
            }
          ]
        }
      ]
    }, {
      "id" => "794f5279-8ed5-5563-9229-3d2573f23051",
      "slug" => "admin/terminator",
      "interpretations" => [
        {
          "id" => "6a04a399-6606-5c51-93fc-14766af0c30c",
          "slug" => "admin/terminator/interpretations/terminator_find",
          "scope" => "public",
          "expressions" => [
            {
              "expression" => "Where is Sarah Connor ?",
              "locale" => "en",
              "solution" => "Where is Sarah Connor ?"
            }
          ]
        }]
    }]
    assert_equal expected, p.full_json_export
  end

  test 'No interpretation alias' do
    expected_expression = 'I want to go to Paris from London'
    interpretation = Interpretation.new(expression: 'I want to go to Paris from London', locale: 'en')
    interpretation.save
    assert_equal expected_expression, interpretation.expression_with_aliases
  end


  test 'Only interpretation alias' do
    expected_expression = '@{route}'

    interpretation = Interpretation.new
    interpretation_alias = InterpretationAlias.new

    interpretation_alias.stubs(:position_start).returns(0)
    interpretation_alias.stubs(:position_end).returns(33)
    interpretation_alias.stubs(:interpretation).returns(interpretation)
    interpretation_alias.stubs(:aliasname).returns('route')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias]
    array.stubs(:order).returns([interpretation_alias])
    array.stubs(:count).returns([interpretation_alias].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end


  test 'Start with interpretation alias' do
    expected_expression = '@{want} to go to Paris from London'

    interpretation = Interpretation.new
    interpretation_alias = InterpretationAlias.new

    interpretation_alias.stubs(:position_start).returns(0)
    interpretation_alias.stubs(:position_end).returns(6)
    interpretation_alias.stubs(:interpretation).returns(interpretation)
    interpretation_alias.stubs(:aliasname).returns('want')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias]
    array.stubs(:order).returns([interpretation_alias])
    array.stubs(:count).returns([interpretation_alias].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end


  test 'End with interpretation alias' do
    expected_expression = 'I want to go to Paris from @{london}'

    interpretation = Interpretation.new
    interpretation_alias = InterpretationAlias.new

    interpretation_alias.stubs(:position_start).returns(27)
    interpretation_alias.stubs(:position_end).returns(33)
    interpretation_alias.stubs(:interpretation).returns(interpretation)
    interpretation_alias.stubs(:aliasname).returns('london')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias]
    array.stubs(:order).returns([interpretation_alias])
    array.stubs(:count).returns([interpretation_alias].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end


  test 'Middle interpretation alias' do
    expected_expression = 'I want to go @{prep-to} Paris from London'

    interpretation = Interpretation.new
    interpretation_alias = InterpretationAlias.new

    interpretation_alias.stubs(:position_start).returns(13)
    interpretation_alias.stubs(:position_end).returns(15)
    interpretation_alias.stubs(:interpretation).returns(interpretation)
    interpretation_alias.stubs(:aliasname).returns('prep-to')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias]
    array.stubs(:order).returns([interpretation_alias])
    array.stubs(:count).returns([interpretation_alias].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end


  test 'Repeated interpretation alias' do
    expected_expression = 'I want to go to @{town_from} from @{town_to}'

    interpretation = Interpretation.new
    interpretation_alias1 = InterpretationAlias.new
    interpretation_alias2 = InterpretationAlias.new

    interpretation_alias1.stubs(:position_start).returns(16)
    interpretation_alias1.stubs(:position_end).returns(21)
    interpretation_alias1.stubs(:interpretation).returns(interpretation)
    interpretation_alias1.stubs(:aliasname).returns('town_from')

    interpretation_alias2.stubs(:position_start).returns(27)
    interpretation_alias2.stubs(:position_end).returns(33)
    interpretation_alias2.stubs(:interpretation).returns(interpretation)
    interpretation_alias2.stubs(:aliasname).returns('town_to')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias1, interpretation_alias2]
    array.stubs(:order).returns([interpretation_alias1, interpretation_alias2])
    array.stubs(:count).returns([interpretation_alias1, interpretation_alias2].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end
end
