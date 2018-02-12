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
    assert_not_equal updated_at_before, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after intent modification' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = agent.intents.first
    intent.description = 'New description'
    assert intent.save
    assert_not_equal updated_at_before, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after intent deletion' do
    agent = Agent.find_by_agentname('weather')
    updated_at_before = agent.updated_at.to_json

    intent = agent.intents.first
    assert intent.destroy
    assert_not_equal updated_at_before, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation addition' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    # Interpretation creation
    interpretation = Interpretation.new(
      expression: 'Good morning John',
      locale: 'en'
    )
    intent = Intent.find_by_intentname('weather_greeting')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation.intent = intent
    assert interpretation.save

    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation modification' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_greeting')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation = intent.interpretations.first
    interpretation.locale = 'fr'
    assert interpretation.save

    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation deletion' do
    agent = Agent.find_by_agentname('weather')
    agent_updated_at_before = agent.updated_at.to_json

    intent = Intent.find_by_intentname('weather_greeting')
    intent_updated_at_before = intent.updated_at.to_json

    interpretation = intent.interpretations.first
    assert interpretation.destroy

    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases addition' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_who')
    interpretation = Interpretation.find_by_expression('Bonjour tout le monde')

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

    assert_not_equal interpretation_updated_at_before, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases modification' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_greeting')
    interpretation = intent.interpretations.first

    interpretation_updated_at_before = interpretation.updated_at.to_json
    intent_updated_at_before         = intent.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    interpretation_alias = interpretation.interpretation_aliases.first
    interpretation_alias.aliasname = 'changed'
    assert interpretation_alias.save

    assert_not_equal interpretation_updated_at_before, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after aliases deletion' do
    agent = Agent.find_by_agentname('weather')
    intent = Intent.find_by_intentname('weather_greeting')
    interpretation = intent.interpretations.first

    interpretation_updated_at_before = interpretation.updated_at.to_json
    intent_updated_at_before         = intent.updated_at.to_json
    agent_updated_at_before          = agent.updated_at.to_json

    interpretation_alias = interpretation.interpretation_aliases.first
    assert interpretation_alias.destroy

    assert_not_equal interpretation_updated_at_before, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at_before, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at_before, agent.reload.updated_at.to_json
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
