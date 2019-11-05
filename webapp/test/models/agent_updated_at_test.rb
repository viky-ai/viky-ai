require 'test_helper'
require 'model_test_helper'

class AgentUpdatedAtTest < ActiveSupport::TestCase

  test 'Change updated_at agent date after interpretation addition' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    interpretation = Interpretation.new(
      interpretation_name: 'greeting',
      description: 'Hello random citizen !'
    )
    interpretation.agent = agent

    assert interpretation.save
    force_reset_model_cache(agent)
    assert_not_equal updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation modification' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    interpretation = agent.interpretations.first
    interpretation.description = 'New description'
    assert interpretation.save
    force_reset_model_cache(agent)
    assert_not_equal updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation deletion' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    interpretation = agent.interpretations.first
    assert interpretation.destroy
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
    interpretation = Interpretation.find_by_interpretation_name('weather_forecast')
    interpretation_updated_at_before = interpretation.updated_at.to_json

    formulation.interpretation = interpretation
    assert formulation.save

    force_reset_model_cache([agent, interpretation])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
  end


  test 'Change updated_at agent date after formulation modification' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    interpretation = Interpretation.find_by_interpretation_name('weather_forecast')
    interpretation_updated_at_before = interpretation.updated_at.to_json

    formulation = interpretation.formulations.last
    formulation.locale = 'fr'
    assert formulation.save

    force_reset_model_cache([agent, interpretation])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
  end


  test 'Change updated_at agent date after formulation deletion' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    interpretation = Interpretation.find_by_interpretation_name('weather_forecast')
    interpretation_updated_at_before = interpretation.updated_at.to_json

    formulation = interpretation.formulations.first
    assert formulation.destroy

    force_reset_model_cache([agent, interpretation])
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases addition' do
    agent = Agent.find_by_agentname('weather')
    interpretation = Interpretation.find_by_interpretation_name('weather_question')
    formulation = Formulation.find_by_expression('Quel temps fera-t-il demain ?')

    formulation_alias = FormulationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    formulation_alias.formulation = formulation
    formulation_alias.formulation_aliasable = interpretation

    formulation_updated_at_before = formulation.updated_at.to_json
    agent_updated_at_before       = agent.updated_at.to_json
    assert formulation_alias.save

    force_reset_model_cache([agent, interpretation, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
    # assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases modification' do
    agent = Agent.find_by_agentname('weather')
    interpretation = Interpretation.find_by_interpretation_name('weather_forecast')
    formulation = interpretation.formulations.last

    formulation_updated_at_before    = formulation.updated_at.to_json
    interpretation_updated_at_before = interpretation.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    formulation_alias = formulation.formulation_aliases.first
    formulation_alias.aliasname = 'changed'
    assert formulation_alias.save

    force_reset_model_cache([agent, interpretation, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases deletion' do
    agent = Agent.find_by_agentname('weather')
    interpretation = Interpretation.find_by_interpretation_name('weather_forecast')
    formulation = interpretation.formulations.last

    formulation_updated_at_before    = formulation.updated_at.to_json
    interpretation_updated_at_before = interpretation.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    formulation_alias = formulation.formulation_aliases.first
    assert formulation_alias.destroy

    force_reset_model_cache([agent, interpretation, formulation])
    assert_not_equal formulation_updated_at_before, formulation.updated_at.to_json
    assert_not_equal interpretation_updated_at_before, interpretation.updated_at.to_json
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


  test 'Change updated_at agent date after delete interpretation in parent' do
    child = create_agent("Agent A")
    interpretation_child = Interpretation.create(
      interpretation_name: 'interpretation_child',
      agent: child
    )
    formulation_child = Formulation.create(
      expression: 'formulation_child',
      locale: 'en',
      interpretation: interpretation_child
    )

    parent = create_agent("Agent B")
    interpretation_parent = Interpretation.create(
      interpretation_name: 'interpretation_parent',
      agent: parent
    )

    assert AgentArc.create(source: child, target: parent)

    assert FormulationAlias.create(
      position_start: 0,
      position_end: 16,
      aliasname: 'inter_parent',
      formulation_id: formulation_child.id,
      formulation_aliasable: interpretation_parent,
      nature: 'type_interpretation'
    )

    force_reset_model_cache([child, interpretation_child])
    updated_at_a = child.updated_at.to_json
    updated_at_interpretation_child = interpretation_child.updated_at.to_json

    assert interpretation_parent.destroy

    force_reset_model_cache([child, interpretation_child])
    assert_not_equal updated_at_interpretation_child, interpretation_child.updated_at.to_json
    assert_not_equal updated_at_a, child.updated_at.to_json
  end


  test 'Change updated_at agent date after delete entities list in parent' do
    child = create_agent("Agent A")
    interpretation_child = Interpretation.create(
      interpretation_name: 'interpretation_child',
      agent: child
    )
    formulation_child = Formulation.create(
      expression: 'formulation_child',
      locale: 'en',
      interpretation: interpretation_child
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
      formulation_id: formulation_child.id,
      formulation_aliasable: entities_list_parent,
      nature: 'type_entities_list'
    )

    force_reset_model_cache([child, interpretation_child])
    updated_at_a = child.updated_at.to_json
    updated_at_interpretation_child = interpretation_child.updated_at.to_json

    assert entities_list_parent.destroy

    force_reset_model_cache([child, interpretation_child])
    assert_not_equal updated_at_interpretation_child, interpretation_child.updated_at.to_json
    assert_not_equal updated_at_a, child.updated_at.to_json
  end
end
