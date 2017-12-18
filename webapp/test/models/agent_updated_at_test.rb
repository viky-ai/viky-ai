require 'test_helper'
require 'model_test_helper'

class AgentUpdatedAtTest < ActiveSupport::TestCase

  test 'Change updated_at agent date after intent update' do
    agent = Agent.find_by_agentname('weather')
    updated_at = agent.updated_at.to_json

    # Intent creation
    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en']
    )
    intent.agent = agent

    assert intent.save
    assert_not_equal updated_at, agent.reload.updated_at.to_json

    # Intent update
    updated_at = agent.updated_at.to_json
    intent.description = 'New description'
    assert intent.save
    assert_not_equal updated_at, agent.reload.updated_at.to_json

    # Intent destroy
    updated_at = agent.updated_at.to_json
    assert intent.destroy
    assert_not_equal updated_at, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation update' do
    agent = Agent.find_by_agentname('weather')

    agent_updated_at = agent.updated_at.to_json

    # Interpretation creation
    interpretation = Interpretation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    intent = Intent.find_by_intentname('weather_greeting')
    intent_updated_at = intent.updated_at.to_json
    interpretation.intent = intent
    assert interpretation.save
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    # Interpretation update
    agent_updated_at  = agent.updated_at.to_json
    intent_updated_at = intent.updated_at.to_json
    interpretation.locale = 'fr'
    assert interpretation.save
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    # Interpretation destroy
    agent_updated_at  = agent.updated_at.to_json
    intent_updated_at = intent.updated_at.to_json
    assert interpretation.destroy
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases update' do

    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_who')
    interpretation = Interpretation.find_by_expression('Bonjour tout le monde')

    # Interpretation creation
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    interpretation_alias.interpretation = interpretation
    interpretation_alias.intent = intent

    interpretation_updated_at = interpretation.updated_at.to_json
    intent_updated_at = intent.updated_at.to_json
    agent_updated_at = agent.updated_at.to_json
    assert interpretation_alias.save

    assert_not_equal interpretation_updated_at, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    # Interpretation update
    interpretation_updated_at = interpretation.updated_at.to_json
    intent_updated_at = intent.updated_at.to_json
    agent_updated_at = agent.updated_at.to_json
    interpretation_alias.aliasname = 'what'
    assert interpretation_alias.save

    assert_not_equal interpretation_updated_at, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    # Interpretation destroy
    interpretation_updated_at = interpretation.updated_at.to_json
    intent_updated_at = intent.updated_at.to_json
    agent_updated_at = agent.updated_at.to_json
    assert interpretation_alias.destroy

    assert_not_equal interpretation_updated_at, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after AgentArc update' do
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    updated_at_a = agent_a.updated_at.to_json
    updated_at_b = agent_b.updated_at.to_json

    arc = AgentArc.create(source: agent_a, target: agent_b)
    assert_not_equal updated_at_a, agent_a.reload.updated_at.to_json
    assert_equal     updated_at_b, agent_b.reload.updated_at.to_json

    updated_at_a = agent_a.updated_at.to_json
    assert arc.destroy

    assert_not_equal updated_at_a, agent_a.reload.updated_at.to_json
    assert_equal     updated_at_b, agent_b.reload.updated_at.to_json
  end
end
