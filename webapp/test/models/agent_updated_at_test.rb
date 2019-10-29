require 'test_helper'
require 'model_test_helper'

class AgentUpdatedAtTest < ActiveSupport::TestCase

  test 'Change updated_at agent date after intent addition' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !'
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


  test 'Change updated_at agent date after formulation addition' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    # Formulation creation
    formulation = Formulation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    formulation.intent = intent
    assert formulation.save

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after formulation modification' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    formulation = intent.formulations.last
    formulation.locale = 'fr'
    assert formulation.save

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after formulation deletion' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_forecast')
    intent_updated_at_before = intent.updated_at.to_json

    formulation = intent.formulations.first
    assert formulation.destroy

    force_reset_model_cache([agent, intent])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases addition' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_question')
    formulation = Formulation.find_by_expression('Quel temps fera-t-il demain ?')

    formulation_alias = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    formulation_alias.formulation = formulation
    formulation_alias.formulation_aliasable = intent

    formulation_updated_at_before = formulation.updated_at.to_json
    agent_updated_at_before       = agent.updated_at.to_json
    assert formulation_alias.save

    force_reset_model_cache([agent, intent, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
    # assert_not_equal intent_updated_at_before, intent.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases modification' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_forecast')
    formulation = intent.formulations.last

    formulation_updated_at_before = formulation.updated_at.to_json
    intent_updated_at_before      = intent.updated_at.to_json
    agent_updated_at_before       = agent.updated_at.to_json

    formulation_alias = formulation.formulation_aliases.first
    formulation_alias.aliasname = 'changed'
    assert formulation_alias.save

    force_reset_model_cache([agent, intent, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases deletion' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_forecast')
    formulation = intent.formulations.last

    formulation_updated_at_before = formulation.updated_at.to_json
    intent_updated_at_before      = intent.updated_at.to_json
    agent_updated_at_before       = agent.updated_at.to_json

    formulation_alias = formulation.formulation_aliases.first
    assert formulation_alias.destroy

    force_reset_model_cache([agent, intent, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
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
      agent: child
    )
    formulation_child = Formulation.create(
      expression: 'formulation_child',
      locale: 'en',
      intent: intent_child
    )

    parent = create_agent("Agent B")
    intent_parent = Intent.create(
      intentname: 'intent_parent',
      agent: parent
    )

    assert AgentArc.create(source: child, target: parent)

    assert FormulationAlias.create(
      position_start: 0,
      position_end: 16,
      aliasname: 'inter_parent',
      formulation_id: formulation_child.id,
      formulation_aliasable: intent_parent,
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
      agent: child
    )
    interpretation_child = Formulation.create(
      expression: 'formulation_child',
      locale: 'en',
      intent: intent_child
    )

    parent = create_agent("Agent B")
    entities_list_parent = EntitiesList.create(
      listname: 'entities_list_parent',
      agent: parent
    )

    assert AgentArc.create(source: child, target: parent)

    assert FormulationAlias.create(
      position_start: 0,
      position_end: 16,
      aliasname: 'inter_parent',
      formulation_id: interpretation_child.id,
      formulation_aliasable: entities_list_parent,
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
