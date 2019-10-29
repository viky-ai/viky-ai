require 'test_helper'
require 'model_test_helper'

class IntentTest < ActiveSupport::TestCase

  test 'Basic intent creation & agent association' do
    assert_equal 2, agents(:weather).intents.count

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
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
    assert_not intent.is_public?
    assert intent.is_private?
  end


  test 'intentname, agent and locales are mandatory' do
    intent = Intent.new(description: 'Hello random citizen !')
    assert_not intent.save

    expected = [
      "Agent must exist",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank"
    ]
    assert_equal expected, intent.errors.full_messages
  end


  test 'Unique intentname per agent' do
    intent = Intent.new(
      intentname: 'hello',
      description: 'Hello random citizen !'
    )
    intent.agent = agents(:weather)
    assert intent.save

    other_intent = Intent.new(
      intentname: 'hello',
      description: 'Another way to greet you...'
    )
    other_intent.agent = agents(:weather)
    assert_not other_intent.save

    expected = ['ID has already been taken']
    assert_equal expected, other_intent.errors.full_messages
  end


  test 'Check intentname minimal length' do
    intent = Intent.new(
      intentname: 'h',
      description: 'Hello random citizen !'
    )
    intent.agent = agents(:weather)
    assert_not intent.save

    expected = ['ID is too short (minimum is 3 characters)']
    assert_equal expected, intent.errors.full_messages
  end


  test 'Check intentname is cleaned' do
    intent = Intent.new(
      intentname: "H3ll-#'!o",
      description: 'Hello random citizen !'
    )
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


  test 'Do not override color at creation if already set' do
    intent = Intent.new(
      intentname: 'greeting',
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
      position: 1,
      agent: agent
    )
    intent_1 = Intent.create(
      intentname: 'intent_1',
      position: 2,
      agent: agent
    )
    intent_2 = Intent.create(
      intentname: 'intent_2',
      position: 3,
      agent: agent
    )

    new_positions = [intent_1.id, intent_2.id, intent_0.id, '132465789']

    Intent.update_positions(agent, [], new_positions)

    force_reset_model_cache([intent_0, intent_1, intent_2])
    assert_equal [3, 2, 1], [intent_1.position, intent_2.position, intent_0.position]
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
    expected = ['Agent does not exist anymore']

    assert_equal expected, weather.errors.full_messages
    assert_equal weather.agent.id, agents(:weather).id
  end


  test 'Move intent without enough quota' do
    Feature.with_quota_enabled do
      intent = intents(:weather_forecast)
      destination_agent = agents(:terminator)

      assert_equal 2, intent.formulations.count
      assert_equal 10, destination_agent.owner.expressions_count

      Quota.stubs(:expressions_limit).returns(11)
      assert_not intent.move_to_agent(destination_agent)

      expected = ["Agent owner does not have enough quota to accept this move"]
      assert_equal expected, intent.errors.full_messages

      Quota.stubs(:expressions_limit).returns(12)
      assert intent.move_to_agent(destination_agent)
    end
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
      visibility: 'is_private'
    )
    weather_loc_intent.agent = dependent_agent
    assert weather_loc_intent.save

    formulation = Formulation.new(
      expression: 'Where is it raining?'
    )
    formulation.intent = weather_loc_intent
    formulation_alias = FormulationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'Where',
      nature: 'type_intent'
    )
    formulation_alias.formulation = formulation
    formulation_alias.formulation_aliasable = current_intent
    assert formulation_alias.save

    expected = [intents(:terminator_find)]
    actual = current_intent.aliased_intents
    assert_equal expected, actual
    assert_not_includes actual, weather_loc_intent
  end


  test 'Adding an formulation trigger an NLP sync' do
    intent = intents(:weather_forecast)
    intent.agent.expects(:sync_nlp).once

    formulation = Formulation.new(
      expression: 'Good morning John',
      locale: 'en',
      intent: intent
    )
    assert formulation.save
  end
end
