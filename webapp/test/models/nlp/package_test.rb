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
          "id"   => intents(:weather_greeting).id,
          "slug" => "admin/weather/weather_greeting",
          "expressions" => [
            {
              "expression" => "Hello @{who}",
              "aliases"    => [
                {
                  "alias"   => "who",
                  "slug"    => "admin/weather/weather_who",
                  "id"      => intents(:weather_who).id,
                  "package" => weather.id
                }
              ],
              "locale"     => "en-US",
              "keep-order" => true,
              "glued"      => true,
              "solution"   => "`greeting.who`"
            },
            {
              "expression" => "Bonjour tout le monde",
              "locale"     => "fr-FR"
            }
          ]
        },
        {
          "id"   => intents(:weather_who).id,
          "slug" => "admin/weather/weather_who",
          "expressions" => [
            {
              "expression" => "world",
              "locale"     => "en-US"
            }
          ]
        }
      ]
    }

    assert_equal expected, JSON.parse(p.generate_json)
  end


  test 'Package generation with locale any' do
    weather = agents(:weather)
    intent = intents(:weather_greeting)
    interpretation = interpretations(:weather_greeting_bonjour)
    interpretation.locale = '*'
    interpretation.save
    intent.interpretations = [interpretation]
    intent.save
    assert intents(:weather_who).destroy

    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id"   => intent.id,
          "slug" => "admin/weather/weather_greeting",
          "expressions" => [
            {
              "expression" => "Bonjour tout le monde",
            }
          ]
        }
      ]
    }
    assert_equal expected, JSON.parse(p.generate_json)
  end


  test 'Validate endpoint' do
    weather = agents(:weather)
    p = Nlp::Package.new(weather)
    endpoint = ENV.fetch('VOQALAPP_NLP_URL') { 'http://localhost:9345' }
    assert_equal endpoint, p.endpoint
  end


  test 'Validate package URL' do
    weather = agents(:weather)
    p = Nlp::Package.new(weather)
    assert_equal "#{p.endpoint}/packages/#{weather.id}", p.url
  end


  test 'No interpretation alias' do
    expected_expression = 'I want to go to Paris from London'
    interpretation = Interpretation.new(expression: 'I want to go to Paris from London', locale: 'en-US')
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
    expected_expression = 'I want to go to @{town} from @{town}'

    interpretation = Interpretation.new
    interpretation_alias1 = InterpretationAlias.new
    interpretation_alias2 = InterpretationAlias.new

    interpretation_alias1.stubs(:position_start).returns(16)
    interpretation_alias1.stubs(:position_end).returns(21)
    interpretation_alias1.stubs(:interpretation).returns(interpretation)
    interpretation_alias1.stubs(:aliasname).returns('town')

    interpretation_alias2.stubs(:position_start).returns(27)
    interpretation_alias2.stubs(:position_end).returns(33)
    interpretation_alias2.stubs(:interpretation).returns(interpretation)
    interpretation_alias2.stubs(:aliasname).returns('town')

    interpretation.stubs(:expression).returns('I want to go to Paris from London')
    array = [interpretation_alias1, interpretation_alias2]
    array.stubs(:order).returns([interpretation_alias1, interpretation_alias2])
    array.stubs(:count).returns([interpretation_alias1, interpretation_alias2].size)
    interpretation.stubs(:interpretation_aliases).returns(array)

    assert_equal expected_expression, interpretation.expression_with_aliases
  end
end
