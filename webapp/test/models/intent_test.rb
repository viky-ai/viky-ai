require 'test_helper'
require 'model_test_helper'

class IntentTest < ActiveSupport::TestCase

  test 'Basic intent creation & agent association' do
    assert_equal 2, agents(:weather).intents.count

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en'],
      visibility: 'is_private'
    )
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 2, intent.position
    assert_equal 'greeting', intent.intentname
    assert_equal agents(:weather).id, intent.agent.id
    assert_equal 3, agents(:weather).intents.count
    assert Intent::AVAILABLE_COLORS.one? { |color| color == intent.color }
    assert_equal 'is_private', intent.visibility
    assert !intent.is_public?
    assert intent.is_private?
  end


  test 'intentname, agent and locales are mandatory' do
    intent = Intent.new(description: 'Hello random citizen !')
    assert !intent.save

    expected = {
      agent: ['must exist'],
      intentname: ['is too short (minimum is 3 characters)', 'can\'t be blank'],
      locales: ['can\'t be blank']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Unique intentname per agent' do
    intent = Intent.new(intentname: 'hello', description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert intent.save

    other_intent = Intent.new(intentname: 'hello', description: 'Another way to greet you...', locales: ['en'])
    other_intent.agent = agents(:weather)
    assert !other_intent.save

    expected = {
      intentname: ['has already been taken']
    }
    assert_equal expected, other_intent.errors.messages
  end


  test 'Check intentname minimal length' do
    intent = Intent.new(intentname: 'h', description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert !intent.save

    expected = {
      intentname: ['is too short (minimum is 3 characters)']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Check intentname is cleaned' do
    intent = Intent.new(intentname: "H3ll-#'!o", description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 'h3ll-o', intent.intentname
  end


  test 'Check intent slug' do
    intent = intents(:weather_forecast)
    original_intentname = intent.intentname
    intent.intentname = 'bonjour'
    assert intent.save

    assert_equal intent.id, Intent.friendly.find(original_intentname).id
  end


  test 'Intent destroy' do
    intent = intents(:weather_forecast)
    intent_id = intent.id

    assert_equal 1, Intent.where(id: intent_id).count
    assert intent.destroy
    assert_equal 0, Intent.where(id: intent_id).count
  end


  test 'Add a locale to an intent' do
    intent = intents(:terminator_find)

    assert_equal %w[en], intent.locales
    intent.locales << 'fr'
    assert intent.save
    assert_equal %w[en fr], intent.locales
  end


  test 'Add an unknown locale to an intent' do
    intent = intents(:weather_forecast)
    intent.locales << 'xx-XX'

    assert !intent.save
    expected = {
      locales: ["unknown 'xx-XX'"]
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Remove a locale from an intent' do
    intent = intents(:weather_forecast)
    assert_equal %w[en fr], intent.locales
    intent.locales -= ['fr']
    assert intent.save
    assert_equal %w[en], intent.locales
  end


  test 'Remove an unknown locale from an intent' do
    intent = intents(:weather_forecast)
    assert_equal %w[en fr], intent.locales
    intent.locales -= ['xx-XX']
    assert intent.save
    assert_equal %w[en fr], intent.locales
  end


  test 'Remove the last locale from an intent' do
    intent = intents(:weather_forecast)
    intent.locales -= ['fr']
    assert intent.save
    intent.locales -= ['en']
    assert !intent.save
    expected = {
        locales: ['can\'t be blank']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Do not override color at creation if already set' do
    intent = Intent.new(
      intentname: 'greeting',
      locales: ['en'],
      color: 'blue'
    )
    intent.agent = agents(:weather)
    assert intent.save
    assert_equal 'blue', intent.color
  end


  test 'Test intent generation' do
    intent = intents(:weather_forecast)
    assert_equal 'admin/weather/interpretations/weather_forecast', intent.slug
  end


  test 'Test update intents positions' do
    agent = agents(:weather_confirmed)
    intent_0 = Intent.create(
      intentname: 'intent_0',
      locales: ['en'],
      position: 0,
      agent: agent
    )
    intent_1 = Intent.create(
      intentname: 'intent_1',
      locales: ['en'],
      position: 1,
      agent: agent
    )
    intent_2 = Intent.create(
      intentname: 'intent_2',
      locales: ['en'],
      position: 2,
      agent: agent
    )

    new_positions = [intent_1.id, intent_2.id, intent_0.id, '132465789']

    Intent.update_positions(agent, [], new_positions)

    force_reset_model_cache([intent_0, intent_1, intent_2])
    assert_equal [2, 1, 0], [intent_1.position, intent_2.position, intent_0.position]
    assert_equal %w(is_private is_private is_private), [intent_1.visibility, intent_2.visibility, intent_0.visibility]
  end


  test 'Move intent to an agent' do
    weather = intents(:terminator_find)
    assert weather.move_to_agent(agents(:weather))
    assert_equal agents(:weather).id, weather.agent.id
    assert_equal 'is_public', weather.visibility
    assert_equal 2, weather.position
  end


  test 'Move intent to an unknown agent' do
    weather = intents(:weather_forecast)
    assert_not weather.move_to_agent(nil)
    expected = {
      agent: ['does not exist']
    }
    assert_equal expected, weather.errors.messages
    assert_equal weather.agent.id, agents(:weather).id
  end

  test 'Get intents that are using the current intent (aliased_intents)' do
    current_intent = intents(:simple_where)
    expected = [intents(:terminator_find)]
    actual = current_intent.aliased_intents
    assert_equal expected, actual
  end

  test 'No intents that are using the current intent (aliased_intents)' do
    current_intent = intents(:weather_forecast)
    expected = []
    actual = current_intent.aliased_intents
    assert_equal expected, actual
  end

  test 'Get intents of another agent that are using the current intent' do
    current_intent = intents(:simple_where)

    dependent_agent = create_agent("Where weather")
    target_agent = agents(:terminator)
    AgentArc.create(source: dependent_agent, target: target_agent)

    weather_loc_intent = Intent.new(
      intentname: 'weather_location',
      description: 'Find locations with similar weather',
      locales: ['en'],
      visibility: 'is_private'
    )
    weather_loc_intent.agent = dependent_agent
    assert weather_loc_intent.save

    interpretation = interpretation = Interpretation.new(
      expression: 'Where is it raining?',
      locale: 'en'
    )
    interpretation.intent = weather_loc_intent
    interpretation_alias = InterpretationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'Where',
      nature: 'type_intent'
    )
    interpretation_alias.interpretation = interpretation
    interpretation_alias.interpretation_aliasable = current_intent
    assert interpretation_alias.save

    expected = [intents(:terminator_find)]
    actual = current_intent.aliased_intents
    assert_equal expected, actual
    assert_not_includes actual, weather_loc_intent
  end

end
