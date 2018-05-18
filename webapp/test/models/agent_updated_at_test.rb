require 'test_helper'
require 'model_test_helper'

class AgentUpdatedAtTest < ActiveSupport::TestCase

  test 'Change updated_at agent date after intent addition' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en']
    )
    intent.agent = agent

    assert intent.save
    force_reset_model_cache(agent)
    assert_not_equal updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after intent modification' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = agent.intents.first
    intent.description = 'New description'
    assert intent.save
    force_reset_model_cache(agent)
    assert_not_equal updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after intent deletion' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = agent.intents.first
    assert intent.destroy
    force_reset_model_cache(agent)
    assert_not_equal updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation addition' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    # Interpretation creation
    interpretation = Interpretation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation.intent = intent
    assert interpretation.save

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation modification' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation = intent.interpretations.last
    interpretation.locale = 'fr'
    assert interpretation.save

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation deletion' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation = intent.interpretations.first
    assert interpretation.destroy

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases addition' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_question')
    interpretation = Interpretation.find_by_expression('Quel temps fera-t-il demain ?')

    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    interpretation_alias.interpretation = interpretation
    interpretation_alias.interpretation_aliasable = intent

    interpretation_updated_at_before = interpretation.updated_at.to_json
    intent_updated_at_before         = intent.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json
    assert interpretation_alias.save

    force_reset_model_cache([agent, intent, interpretation])
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
    # assert_not_equal intent_updated_at_before, intent.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases modification' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_forecast')
    interpretation = intent.interpretations.last

    interpretation_updated_at_before = interpretation.updated_at.to_json
    intent_updated_at_before         = intent.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    interpretation_alias = interpretation.interpretation_aliases.first
    interpretation_alias.aliasname = 'changed'
    assert interpretation_alias.save

    force_reset_model_cache([agent, intent, interpretation])
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases deletion' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_forecast')
    interpretation = intent.interpretations.last

    interpretation_updated_at_before = interpretation.updated_at.to_json
    intent_updated_at_before         = intent.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    interpretation_alias = interpretation.interpretation_aliases.first
    assert interpretation_alias.destroy

    force_reset_model_cache([agent, intent, interpretation])
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after AgentArc update' do
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    updated_at_a = agent_a.updated_at.to_json
    updated_at_b = agent_b.updated_at.to_json

    arc = AgentArc.create(source: agent_a, target: agent_b)

    force_reset_model_cache([agent_a, agent_b])
    assert_not_equal updated_at_a, agent_a.updated_at.to_json
    assert_equal     updated_at_b, agent_b.updated_at.to_json

    updated_at_a = agent_a.updated_at.to_json
    assert arc.destroy

    force_reset_model_cache([agent_a, agent_b])
    assert_not_equal updated_at_a, agent_a.updated_at.to_json
    assert_equal     updated_at_b, agent_b.updated_at.to_json
  end


  test 'Change updated_at agent date after delete intent in parent' do
    child = create_agent("Agent A")
    intent_child = Intent.create(
      intentname: 'intent_child',
      locales: ['en'],
      agent: child
    )
    interpretation_child = Interpretation.create(
      expression: 'interpretation_child',
      locale: 'en',
      intent: intent_child
    )

    parent = create_agent("Agent B")
    intent_parent = Intent.create(
      intentname: 'intent_parent',
      locales: ['en'],
      agent: parent
    )

    assert AgentArc.create(source: child, target: parent)

    assert InterpretationAlias.create(
      position_start: 0,
      position_end: 16,
      aliasname: 'inter_parent',
      interpretation_id: interpretation_child.id,
      interpretation_aliasable: intent_parent,
      nature: 'type_intent'
    )

    force_reset_model_cache([child, intent_child])
    updated_at_a = child.updated_at.to_json
    updated_at_intent_child = intent_child.updated_at.to_json

    assert intent_parent.destroy

    force_reset_model_cache([child, intent_child])
    assert_not_equal updated_at_intent_child, intent_child.updated_at.to_json
    assert_not_equal updated_at_a, child.updated_at.to_json
  end


  test 'Change updated_at agent date after delete entities list in parent' do
    child = create_agent("Agent A")
    intent_child = Intent.create(
      intentname: 'intent_child',
      locales: ['en'],
      agent: child
    )
    interpretation_child = Interpretation.create(
      expression: 'interpretation_child',
      locale: 'en',
      intent: intent_child
    )

    parent = create_agent("Agent B")
    entities_list_parent = EntitiesList.create(
      listname: 'entities_list_parent',
      agent: parent
    )

    assert AgentArc.create(source: child, target: parent)

    assert InterpretationAlias.create(
      position_start: 0,
      position_end: 16,
      aliasname: 'inter_parent',
      interpretation_id: interpretation_child.id,
      interpretation_aliasable: entities_list_parent,
      nature: 'type_entities_list'
    )

    force_reset_model_cache([child, intent_child])
    updated_at_a = child.updated_at.to_json
    updated_at_intent_child = intent_child.updated_at.to_json

    assert entities_list_parent.destroy

    force_reset_model_cache([child, intent_child])
    assert_not_equal updated_at_intent_child, intent_child.updated_at.to_json
    assert_not_equal updated_at_a, child.updated_at.to_json
  end
end
