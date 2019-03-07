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
              "expression"    => "Quel temps fera-t-il demain ?",
              "id"            => interpretations(:weather_forecast_demain).id,
              "locale"        => "fr",
              "solution"      => "Quel temps fera-t-il demain ?",
              "glue-distance" => 20
            },
            {
              "expression" => "@{question} @{when} ?",
              "id"         => interpretations(:weather_forecast_tomorrow).id,
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
              "locale"         => "en",
              "keep-order"     => true,
              "glue-distance" => 0,
              "solution"       => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id"    => intents(:weather_question).id,
          "slug"  => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression"    => "What the weather like",
              "id"            => interpretations(:weather_question_like).id,
              "locale"        => "en",
              "solution"      => "What the weather like",
              "glue-distance" => 20
            }
          ]
        },
        {
          "id"       => entities_lists(:weather_conditions).id,
          "slug"     => "admin/weather/entities_lists/weather_conditions",
          'scope'    => 'public',
          "expressions" => [
            {
              "expression"    => "pluie",
              "id"            => entities(:weather_raining).id,
              "locale"        => "fr",
              "solution"      => "pluie",
              "keep-order"    => true,
              "glue-distance" => 0,
              "glue-strength" => "punctuation",
            },
            {
              "expression" => "rain",
              "id"         => entities(:weather_raining).id,
              "locale"     => "en",
              "solution"   => "pluie",
              "keep-order" => true,
              "glue-distance" => 0,
              "glue-strength" => "punctuation",
            },
            {
              "expression" => "sun",
              "id"         => entities(:weather_sunny).id,
              "locale"     => "en",
              "solution"   => "sun",
              "keep-order" => true,
              "glue-distance" => 0,
              "glue-strength" => "punctuation",
            },
            {
              "expression" => "soleil",
              "id"         => entities(:weather_sunny).id,
              "locale"     => "fr",
              "solution"   => "sun",
              "keep-order" => true,
              "glue-distance" => 0,
              "glue-strength" => "punctuation",
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
              "id"         => entities(:weather_dates_today).id,
              "locale"     => "fr",
              "solution"   => "`{\"date\": \"today\"}`",
              "keep-order" => true,
              "glue-distance" => 20
            },
            {
              "expression" => "tout à l'heure",
              "id"         => entities(:weather_dates_today).id,
              "locale"     => "fr",
              "solution"   => "`{\"date\": \"today\"}`",
              "keep-order" => true,
              "glue-distance" => 20
            },
            {
              "expression" => "today",
              "id"         => entities(:weather_dates_today).id,
              "locale"     => "en",
              "solution"   => "`{\"date\": \"today\"}`",
              "keep-order" => true,
              "glue-distance" => 20
            },
            {
              "expression" => "tomorrow",
              "id"         => entities(:weather_dates_tomorrow).id,
              "solution"   => "`{\"date\": \"tomorrow\"}`",
              "keep-order" => true,
              "glue-distance" => 20
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
              "expression"    => "Quel temps fera-t-il demain ?",
              "id"            => interpretations(:weather_forecast_demain).id,
              "solution"      => "Quel temps fera-t-il demain ?",
              "glue-distance" => 20
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
              "expression"    => "What the weather like",
              "id"            => interpretations(:weather_question_like).id,
              "locale"        => "en",
              "solution"      => "What the weather like",
              "glue-distance" => 20
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
          "id"          => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
          "slug"        => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
          "scope"       => "hidden",
          "expressions" => [
            {
              "expression" => "@{question}",
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question",
                  "id"      => intents(:weather_question).id,
                  "package" => weather.id
                }
              ],
              "keep-order"    => true,
              "glue-distance" => 0
            },
            {
              "expression" => "@{question} @{question_recursive}",
              "id"         => interpretations(:weather_forecast_tomorrow).id,
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
              ],
              "keep-order"    => true,
              "glue-distance" => 0
            }
          ]
        },
        {
          "id"    => intents(:weather_forecast).id,
          "slug"  => "admin/weather/interpretations/weather_forecast",
          'scope' => 'public',
          "expressions" => [
            {
              "expression"     => "Quel temps fera-t-il demain ?",
              "id"             => interpretations(:weather_forecast_demain).id,
              "locale"         => "fr",
              "solution"       => "Quel temps fera-t-il demain ?",
              "glue-distance"  => 20
            },
            {
              "expression" => "@{question} tomorrow ?",
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              "aliases"    => [
                {
                  "alias"   => "question",
                  "slug"    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  "id"      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  "package" => weather.id
                }
              ],
              "locale"        => "en",
              "keep-order"    => true,
              "glue-distance" => 0,
              "solution"      => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id"    => intents(:weather_question).id,
          "slug"  => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression"    => "What the weather like",
              "id"            => interpretations(:weather_question_like).id,
              "locale"        => "en",
              "solution"      => "What the weather like",
              "glue-distance" => 20
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
              'id'         => interpretations(:weather_forecast_tomorrow).id,
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => 'admin/weather/interpretations/weather_question',
                  'id'      => intents(:weather_question).id,
                  'package' => weather.id
                }
              ],
              "keep-order"    => true,
              "glue-distance" => 0
            },
            {
              'expression' => '@{question} @{question_recursive}',
              'id'         => interpretations(:weather_forecast_tomorrow).id,
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
              ],
              "keep-order"    => true,
              "glue-distance" => 0
            },
            {
              'expression'  => '@{question} @{question_recursive}',
              'id'          =>  interpretations(:weather_forecast_tomorrow).id,
              'aliases'     => [
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
              ],
              "keep-order"    => true,
              "glue-distance" => 0
            }
          ]
        },
        {
          'id'    => intents(:weather_forecast).id,
          'slug'  => 'admin/weather/interpretations/weather_forecast',
          'scope' => 'public',
          'expressions' => [
            {
              'expression'    => 'Quel temps fera-t-il demain ?',
              'id'            => interpretations(:weather_forecast_demain).id,
              'locale'        => 'fr',
              'solution'      => "Quel temps fera-t-il demain ?",
              "glue-distance" => 20
            },
            {
              'expression' => '@{question} tomorrow ?',
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => "admin/weather/interpretations/weather_question_#{ialias.id}_recursive",
                  'id'      => "#{intents(:weather_question).id}_#{ialias.id}_recursive",
                  'package' => weather.id
                }
              ],
              'locale'        => 'en',
              'keep-order'    => true,
              'glue-distance' => 0,
              'solution'      => '`forecast.tomorrow`'
            }
          ]
        },
        {
          'id'    => intents(:weather_question).id,
          'slug'  => 'admin/weather/interpretations/weather_question',
          'scope' => 'public',
          'expressions' => [
            {
              'expression'    => 'What the weather like',
              "id"            => interpretations(:weather_question_like).id,
              'locale'        => 'en',
              'solution'      => "What the weather like",
              "glue-distance" => 20
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
              'expression'    => 'Quel temps fera-t-il demain ?',
              'id'            => interpretations(:weather_forecast_demain).id,
              'locale'        => 'fr',
              'solution'      => 'Quel temps fera-t-il demain ?',
              "glue-distance" => 20
            },
            {
              'expression' => '@{question} tomorrow ?',
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'slug'    => 'admin/weather/interpretations/weather_question',
                  'id'      => intents(:weather_question).id,
                  'package' => weather.id
                }
              ],
              'locale'        => 'en',
              'keep-order'    => true,
              'glue-distance' => 0,
              'solution'      => '`forecast.tomorrow`'
            },
            {
              'expression' => '@{question} tomorrow ?',
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              'aliases'    => [
                {
                  'alias'   => 'question',
                  'type'    => 'any'
                }
              ],
              'locale'        => 'en',
              'keep-order'    => true,
              'glue-distance' => 0,
              'solution'      => '`forecast.tomorrow`'
            }
          ]
        },
        {
          'id'    => intents(:weather_question).id,
          'slug'  => 'admin/weather/interpretations/weather_question',
          'scope' => 'public',
          'expressions' => [
            {
              'expression'    => 'What the weather like',
              "id"            => interpretations(:weather_question_like).id,
              'locale'        => 'en',
              'solution'      => "What the weather like",
              "glue-distance" => 20
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
              "expression"    => "Quel temps fera-t-il demain ?",
              "id"            => interpretations(:weather_forecast_demain).id,
              "locale"        => "fr",
              "solution"      => "Quel temps fera-t-il demain ?",
              "glue-distance" => 20
            },
            {
              "expression" => "@{question} tomorrow ?",
              "id"         => interpretations(:weather_forecast_tomorrow).id,
              "aliases" => [
                {
                  "alias" => "question",
                  "slug" => "admin/weather/interpretations/weather_question",
                  "id" => intents(:weather_question).id,
                  "package" => weather.id
                }
              ],
              "locale"        => "en",
              "keep-order"    => true,
              "glue-distance" => 0,
              "solution"      => "`forecast.tomorrow`"
            }
          ]
        },
        {
          "id" => intents(:weather_question).id,
          "slug" => "admin/weather/interpretations/weather_question",
          'scope' => 'public',
          "expressions" => [
            {
              "expression"    => "What the weather like",
              "id"            => interpretations(:weather_question_like).id,
              "locale"        => "en",
              "solution"      => "What the weather like",
              "glue-distance" => 20
            }
          ]
        }
      ]
    }, {
      "id" => "794f5279-8ed5-5563-9229-3d2573f23051",
      "slug" => "admin/terminator",
      "interpretations" => [
        {
          "id"=>"997928ef-e905-535e-b36a-2f64b7349c3b",
          "slug"=>"admin/terminator/interpretations/simple_where",
          "scope"=>"private",
          "expressions"=>  [
              {
                "expression"    =>"Find",
                "id"            =>"55495e57-e816-5b14-b665-2205a6ccf263",
                "locale"        =>"en",
                "solution"      =>"Find",
                "glue-distance" => 20
              }
            ]
        },
        {
          "id" => "6a04a399-6606-5c51-93fc-14766af0c30c",
          "slug" => "admin/terminator/interpretations/terminator_find",
          "scope" => "public",
          "expressions" => [
            {
              "expression" => "@{find} Sarah Connor",
              "id"=>"dfcf3b39-2f54-5847-b717-a8040707f232",
              "aliases"=> [
                {
                  "alias"   => "find",
                  "slug"    => "admin/terminator/interpretations/simple_where",
                  "id"      => "997928ef-e905-535e-b36a-2f64b7349c3b",
                  "package" => "794f5279-8ed5-5563-9229-3d2573f23051"
                }
              ],
              "locale"        =>"en",
              "glue-distance" => 20
            },
            {
              "expression"    => "Where is Sarah Connor ?",
              "id"            => interpretations(:terminator_find_sarah).id,
              "locale"        => "en",
              "solution"      => "Where is Sarah Connor ?",
              "glue-distance" => 20
            }
          ]
        },
        {

          "id" => "1f45c98f-b39b-5a8b-a4a7-8379bea19f0a",
          "slug" => "admin/terminator/entities_lists/terminator_targets",
          "scope" => "private",
          "expressions" => []
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

  test 'Package generation with regex type' do
    agent = agents(:terminator)
    interpretation = interpretations(:terminator_find_sarah)
    assert entities_lists(:terminator_targets).destroy
    assert interpretations(:terminator_where).destroy
    assert intents(:simple_where).destroy

    regex_alias = InterpretationAlias.new(
      position_start: 9,
      position_end: 21,
      aliasname: 'name',
      interpretation_id: interpretation.id,
      nature: 'type_regex',
      reg_exp: '[A-Za-z,-]'
    )
    assert regex_alias.save

    package = Nlp::Package.new(agent)
    expected = {
      "id"   => agent.id,
      "slug" => "admin/terminator",
      "interpretations" => [
        {
          "id"    => interpretation.intent.id,
          "slug"  => "admin/terminator/interpretations/terminator_find",
          'scope' => 'public',
          "expressions" => [
            {
              "expression" => "Where is @{name} ?",
              "id"=>"223bbbb3-e8cf-5063-8d1a-b8c4c168bda4",
              "aliases"    => [
                {
                  "alias"   => "name",
                  "type"    => "regex",
                  "regex"   => "[A-Za-z,-]"
                }
              ],
              "locale"        => "en",
              "glue-distance" => 20
            }
          ]
        }
      ]
    }
    assert_equal expected, package.generate_json
  end

end
