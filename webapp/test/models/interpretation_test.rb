require 'test_helper'
require 'model_test_helper'

class InterpretationTest < ActiveSupport::TestCase

  test 'Basic interpretation creation & agent association' do
    assert_equal 2, agents(:weather).interpretations.count

    interpretation = Interpretation.new(
      interpretation_name: 'greeting',
      description: 'Hello random citizen !',
      visibility: 'is_private'
    )
    interpretation.agent = agents(:weather)
    assert interpretation.save

    assert_equal 2, interpretation.position
    assert_equal 'greeting', interpretation.interpretation_name
    assert_equal agents(:weather).id, interpretation.agent.id
    assert_equal 3, agents(:weather).interpretations.count
    assert Interpretation::AVAILABLE_COLORS.one? { |color| color == interpretation.color }
    assert_equal 'is_private', interpretation.visibility
    assert_not interpretation.is_public?
    assert interpretation.is_private?
  end


  test 'interpretation_name, agent and locales are mandatory' do
    interpretation = Interpretation.new(description: 'Hello random citizen !')
    assert_not interpretation.save

    expected = [
      "Agent must exist",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank"
    ]
    assert_equal expected, interpretation.errors.full_messages
  end


  test 'Unique interpretation_name per agent' do
    interpretation = Interpretation.new(
      interpretation_name: 'hello',
      description: 'Hello random citizen !'
    )
    interpretation.agent = agents(:weather)
    assert interpretation.save

    other_interpretation = Interpretation.new(
      interpretation_name: 'hello',
      description: 'Another way to greet you...'
    )
    other_interpretation.agent = agents(:weather)
    assert_not other_interpretation.save

    expected = ['ID has already been taken']
    assert_equal expected, other_interpretation.errors.full_messages
  end


  test 'Check interpretation_name minimal length' do
    interpretation = Interpretation.new(
      interpretation_name: 'h',
      description: 'Hello random citizen !'
    )
    interpretation.agent = agents(:weather)
    assert_not interpretation.save

    expected = ['ID is too short (minimum is 3 characters)']
    assert_equal expected, interpretation.errors.full_messages
  end


  test 'Check interpretation_name is cleaned' do
    interpretation = Interpretation.new(
      interpretation_name: "H3ll-#'!o",
      description: 'Hello random citizen !'
    )
    interpretation.agent = agents(:weather)
    assert interpretation.save

    assert_equal 'h3ll-o', interpretation.interpretation_name
  end


  test 'Check interpretation slug' do
    interpretation = interpretations(:weather_forecast)
    original_interpretation_name = interpretation.interpretation_name
    interpretation.interpretation_name = 'bonjour'
    assert interpretation.save

    assert_equal interpretation.id, Interpretation.friendly.find(original_interpretation_name).id
  end


  test 'interpretation destroy' do
    interpretation = interpretations(:weather_forecast)
    interpretation_id = interpretation.id

    assert_equal 1, Interpretation.where(id: interpretation_id).count
    assert interpretation.destroy
    assert_equal 0, Interpretation.where(id: interpretation_id).count
  end


  test 'Do not override color at creation if already set' do
    interpretation = Interpretation.new(
      interpretation_name: 'greeting',
      color: 'blue'
    )
    interpretation.agent = agents(:weather)
    assert interpretation.save
    assert_equal 'blue', interpretation.color
  end


  test 'Test interpretation generation' do
    interpretation = interpretations(:weather_forecast)
    assert_equal 'admin/weather/interpretations/weather_forecast', interpretation.slug
  end


  test 'Test update interpretations positions' do
    agent = agents(:weather_confirmed)
    interpretation_0 = Interpretation.create(
      interpretation_name: 'interpretation_0',
      position: 1,
      agent: agent
    )
    interpretation_1 = Interpretation.create(
      interpretation_name: 'interpretation_1',
      position: 2,
      agent: agent
    )
    interpretation_2 = Interpretation.create(
      interpretation_name: 'interpretation_2',
      position: 3,
      agent: agent
    )

    new_positions = [interpretation_1.id, interpretation_2.id, interpretation_0.id, '132465789']

    Interpretation.update_positions(agent, [], new_positions)

    force_reset_model_cache([interpretation_0, interpretation_1, interpretation_2])
    assert_equal [3, 2, 1], [interpretation_1.position, interpretation_2.position, interpretation_0.position]
    assert_equal %w(is_private is_private is_private), [interpretation_1.visibility, interpretation_2.visibility, interpretation_0.visibility]
  end


  test 'Move interpretation to an agent' do
    weather = interpretations(:terminator_find)
    assert weather.move_to_agent(agents(:weather))
    assert_equal agents(:weather).id, weather.agent.id
    assert_equal 'is_public', weather.visibility
    assert_equal 2, weather.position
  end


  test 'Move interpretation to an unknown agent' do
    weather = interpretations(:weather_forecast)
    assert_not weather.move_to_agent(nil)
    expected = ['Agent does not exist anymore']

    assert_equal expected, weather.errors.full_messages
    assert_equal weather.agent.id, agents(:weather).id
  end


  test 'Move interpretation without enough quota' do
    Feature.with_quota_enabled do
      interpretation = interpretations(:weather_forecast)
      destination_agent = agents(:terminator)

      assert_equal 2, interpretation.formulations.count
      assert_equal 10, destination_agent.owner.expressions_count

      Quota.stubs(:expressions_limit).returns(11)
      assert_not interpretation.move_to_agent(destination_agent)

      expected = ["Agent owner does not have enough quota to accept this move"]
      assert_equal expected, interpretation.errors.full_messages

      Quota.stubs(:expressions_limit).returns(12)
      assert interpretation.move_to_agent(destination_agent)
    end
  end


  test 'Get interpretations that are using the current interpretation (aliased_interpretations)' do
    current_interpretation = interpretations(:simple_where)
    expected = [interpretations(:terminator_find)]
    actual = current_interpretation.aliased_interpretations
    assert_equal expected, actual
  end


  test 'No interpretations that are using the current interpretation (aliased_interpretations)' do
    current_interpretation = interpretations(:weather_forecast)
    expected = []
    actual = current_interpretation.aliased_interpretations
    assert_equal expected, actual
  end


  test 'Get interpretations of another agent that are using the current interpretation' do
    current_interpretation = interpretations(:simple_where)

    dependent_agent = create_agent("Where weather")
    target_agent = agents(:terminator)
    AgentArc.create(source: dependent_agent, target: target_agent)

    weather_loc_interpretation = Interpretation.new(
      interpretation_name: 'weather_location',
      description: 'Find locations with similar weather',
      visibility: 'is_private'
    )
    weather_loc_interpretation.agent = dependent_agent
    assert weather_loc_interpretation.save

    formulation = Formulation.new(
      expression: 'Where is it raining?'
    )
    formulation.interpretation = weather_loc_interpretation
    formulation_alias = FormulationAlias.new(
      position_start: 0,
      position_end: 5,
      aliasname: 'Where',
      nature: 'type_interpretation'
    )
    formulation_alias.formulation = formulation
    formulation_alias.formulation_aliasable = current_interpretation
    assert formulation_alias.save

    expected = [interpretations(:terminator_find)]
    actual = current_interpretation.aliased_interpretations
    assert_equal expected, actual
    assert_not_includes actual, weather_loc_interpretation
  end


  test 'Adding an formulation trigger an NLP sync' do
    interpretation = interpretations(:weather_forecast)
    interpretation.agent.expects(:sync_nlp).once

    formulation = Formulation.new(
      expression: 'Good morning John',
      locale: 'en',
      interpretation: interpretation
    )
    assert formulation.save
  end
end
