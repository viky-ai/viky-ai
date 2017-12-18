require 'test_helper'
require 'model_test_helper'

class AgentUpdatedAtTest < ActiveSupport::TestCase

  test 'Change updated_at agent date after intent update' do
    agent = agents(:weather)
    updated_at = agent.reload.updated_at.to_json

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en']
    )
    intent.agent = agents(:weather)
    sleep 0.1
    assert intent.save
    assert_not_equal updated_at, agent.reload.updated_at.to_json

    updated_at = agent.reload.updated_at.to_json
    intent.description = 'New description'
    sleep 0.1
    assert intent.save
    assert_not_equal updated_at, agent.reload.updated_at.to_json

    updated_at = agent.reload.updated_at.to_json
    sleep 0.1
    assert intent.destroy
    assert_not_equal updated_at, agent.reload.updated_at.to_json
  end


  test 'Change updated_at agent date after interpretation update' do
    agent = agents(:weather)
    agent_updated_at = agent.reload.updated_at.to_json

    interpretation = Interpretation.new(expression: 'Good morning John', locale: 'en')
    intent = intents(:weather_greeting)
    interpretation.intent = intent
    intent_updated_at = intent.reload.updated_at.to_json

    sleep 0.1
    assert interpretation.save
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    agent_updated_at = agent.reload.updated_at.to_json
    intent_updated_at = intent.reload.updated_at.to_json
    interpretation.locale = 'fr'
    sleep 0.1
    assert interpretation.save
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    agent_updated_at = agent.reload.updated_at.to_json
    intent_updated_at = intent.reload.updated_at.to_json
    sleep 0.1
    assert interpretation.destroy
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    interpretation = Interpretation.new(expression: 'Good morning John', locale: 'en')
    interpretation.save
  end


  test 'Change updated_at agent date after aliases update' do
    interpretation_alias = InterpretationAlias.new(
      position_start: 8,
      position_end: 21,
      aliasname: 'who'
    )
    interpretation = interpretations(:weather_greeting_bonjour)
    interpretation_alias.interpretation = interpretation
    intent = intents(:weather_who)
    agent = agents(:weather)
    interpretation_alias.intent = intent

    interpretation_updated_at = interpretation.reload.updated_at.to_json
    intent_updated_at = intent.reload.updated_at.to_json
    agent_updated_at = agent.reload.updated_at.to_json
    sleep 0.1
    assert interpretation_alias.save
    assert_not_equal interpretation_updated_at, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    interpretation_updated_at = interpretation.reload.updated_at.to_json
    intent_updated_at = intent.reload.updated_at.to_json
    agent_updated_at = agent.reload.updated_at.to_json
    interpretation_alias.aliasname = 'what'
    sleep 0.1
    assert interpretation_alias.save
    assert_not_equal interpretation_updated_at, interpretation.reload.updated_at.to_json
    assert_not_equal intent_updated_at, intent.reload.updated_at.to_json
    assert_not_equal agent_updated_at, agent.reload.updated_at.to_json

    interpretation_updated_at = interpretation.reload.updated_at.to_json
    intent_updated_at = intent.reload.updated_at.to_json
    agent_updated_at = agent.reload.updated_at.to_json
    sleep 0.1
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
    assert_equal updated_at_b, agent_b.reload.updated_at.to_json

    updated_at_a = agent_a.reload.updated_at.to_json
    sleep 0.1
    assert arc.destroy
    assert_not_equal updated_at_a, agent_a.reload.updated_at.to_json
    assert_equal updated_at_b, agent_b.reload.updated_at.to_json
  end
end
