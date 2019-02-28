require 'test_helper'
require 'model_test_helper'

class AgentDuplicateTest < ActiveSupport::TestCase

  test 'Duplicate a standalone agent' do
    agent = Agent.create(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription",
      visibility: 'is_public',
      memberships: [
         Membership.new(user_id: users(:admin).id, rights: 'all')
      ],
      created_at: '2017-01-02 00:00:00',
      updated_at: '2017-01-02 00:00:00'
    )
    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate

    assert_nil new_agent.id
    assert_equal "#{agent.name} [COPY]", new_agent.name
    assert_equal agent.description, new_agent.description
    assert_equal agent.color, new_agent.color
    assert_equal "#{agent.agentname}-copy", new_agent.agentname
    assert_equal 'is_private', new_agent.visibility
    assert_not_equal agent.api_token, new_agent.api_token
    assert_nil new_agent.owner

    assert new_agent.save
    assert_not_equal agent.id, new_agent.id
    assert_equal users(:admin).id, new_agent.owner.id
    assert_not_equal agent.created_at, new_agent.created_at
    assert_not_equal agent.updated_at, new_agent.updated_at

    another_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert_equal "#{agent.name} [COPY 1]", another_agent.name
    assert_equal "#{agent.agentname}-copy-1", another_agent.agentname
  end


  test 'Duplicate public agent' do
    agent = agents(:terminator)
    current_user = users(:show_on_agent_weather)
    new_agent = AgentDuplicator.new(agent, current_user).duplicate
    assert new_agent.save
    assert_equal current_user.id, new_agent.owner.id

    another_agent = AgentDuplicator.new(agent, current_user).duplicate
    assert another_agent.save
  end


  test 'Duplicate keeps track of source agent' do
    agent = agents(:terminator)
    current_user = users(:show_on_agent_weather)
    new_agent = AgentDuplicator.new(agent, current_user).duplicate
    assert new_agent.save

    id   = agent.id
    slug = agent.slug
    assert_equal id, new_agent.source_agent['id']
    assert_equal slug, new_agent.source_agent['slug']

    assert agent.destroy
    assert_equal id, new_agent.source_agent['id']
    assert_equal slug, new_agent.source_agent['slug']
  end


  test 'Duplicate agent with direct relations' do
    agent = agents(:terminator)
    parent1_agent = create_agent('Parent1 agent')
    parent2_agent = create_agent('Parent2 agent')
    assert AgentArc.create(source: agent, target: parent1_agent)
    assert AgentArc.create(source: agent, target: parent2_agent)
    child_agent = create_agent('Child agent')
    assert AgentArc.create(source: child_agent, target: agent)

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate

    assert new_agent.save
    assert_equal agent.readme.content, new_agent.readme.content
    assert_not_equal agent.readme.id, new_agent.readme.id

    assert_equal agent.entities_lists.first.listname, new_agent.entities_lists.first.listname
    assert_not_equal agent.entities_lists.first.id, new_agent.entities_lists.first.id

    assert_equal agent.intents.first.intentname, new_agent.intents.first.intentname
    assert_not_equal agent.intents.first.id, new_agent.intents.first.id

    assert_not_equal agent.out_arcs.first.source.id, new_agent.out_arcs.first.source.id
    assert_equal agent.out_arcs.first.target.id, new_agent.out_arcs.first.target.id

    assert new_agent.in_arcs.empty?
  end


  test 'Duplicate agent deeps relations' do
    agent = agents(:weather)
    assert InterpretationAlias.create(
      position_start: 31,
      position_end: 32,
      aliasname: 'number',
      interpretation: interpretations(:weather_forecast_tomorrow),
      nature: 'type_number'
    )
    assert InterpretationAlias.create(
      position_start: 35,
      position_end: 40,
      aliasname: 'regex',
      interpretation: interpretations(:weather_forecast_tomorrow),
      nature: 'type_regex',
      reg_exp: 'A'
    )

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert new_agent.save

    assert_equal agent.entities_lists.size, new_agent.entities_lists.size
    entities_list = agent.entities_lists.zip(new_agent.entities_lists)
    entities_list.each do |elist_agent, elist_new_agent|
      assert_equal elist_agent.listname, elist_new_agent.listname
      assert_equal elist_agent.visibility, elist_new_agent.visibility
      assert_not_equal elist_agent.id, elist_new_agent.id
      entities = elist_agent.entities.zip(elist_new_agent.entities)
      entities.each do |entity_agent, entity_new_agent|
        assert_equal entity_agent.terms, entity_new_agent.terms
        assert entity_agent.solution == entity_new_agent.solution
        assert_equal entity_agent.auto_solution_enabled, entity_new_agent.auto_solution_enabled
        assert_not_equal entity_agent.id, entity_new_agent.id
        assert_not_equal entity_agent.entities_list.id, entity_new_agent.entities_list.id
      end

      assert_equal elist_agent.interpretation_aliases.size, elist_new_agent.interpretation_aliases.size
      aliases = elist_agent.interpretation_aliases.zip(elist_new_agent.interpretation_aliases)
      aliases.each do |alias_agent, alias_new_agent|
        assert_equal alias_agent.aliasname, alias_new_agent.aliasname
        assert_equal alias_agent.position_start, alias_new_agent.position_start
        assert_equal alias_agent.position_end, alias_new_agent.position_end
        assert_equal alias_agent.nature, alias_new_agent.nature
        assert_equal alias_agent.is_list, alias_new_agent.is_list
        assert_equal alias_agent.any_enabled, alias_new_agent.any_enabled
        assert_not_equal alias_agent.id, alias_new_agent.id
        assert_not_equal alias_agent.interpretation.id, alias_new_agent.interpretation.id
        assert_not_equal alias_agent.interpretation_aliasable.id, alias_new_agent.interpretation_aliasable.id
      end
    end


    assert_equal agent.intents.size, new_agent.intents.size
    intents = agent.intents.zip(new_agent.intents)
    intents.each do |intent_agent, intent_new_agent|
      assert_equal intent_agent.intentname, intent_new_agent.intentname
      assert_equal intent_agent.visibility, intent_new_agent.visibility
      assert_not_equal intent_agent.id, intent_new_agent.id

      assert_equal intent_agent.interpretations.size, intent_new_agent.interpretations.size
      interpretations = intent_agent.interpretations.zip(intent_new_agent.interpretations)
      interpretations.each do |inter_agent, inter_new_agent|
        assert_equal inter_agent.expression, inter_new_agent.expression
        assert_equal inter_agent.locale, inter_new_agent.locale
        assert_equal inter_agent.keep_order, inter_new_agent.keep_order
        assert_equal inter_agent.glued, inter_new_agent.glued
        assert_equal inter_agent.auto_solution_enabled, inter_new_agent.auto_solution_enabled
        assert inter_agent.solution == inter_new_agent.solution
        assert_not_equal inter_agent.id, inter_new_agent.id
        assert_not_equal inter_agent.intent.id, inter_new_agent.intent.id

        assert_equal inter_agent.interpretation_aliases.size, inter_new_agent.interpretation_aliases.size
        aliases = inter_agent.interpretation_aliases.zip(inter_new_agent.interpretation_aliases)
        aliases.each do |alias_agent, alias_new_agent|
          assert_equal alias_agent.aliasname, alias_new_agent.aliasname
          assert_equal alias_agent.position_start, alias_new_agent.position_start
          assert_equal alias_agent.position_end, alias_new_agent.position_end
          assert_equal alias_agent.nature, alias_new_agent.nature
          assert_equal alias_agent.is_list, alias_new_agent.is_list
          assert_equal alias_agent.any_enabled, alias_new_agent.any_enabled
          assert_not_equal alias_agent.id, alias_new_agent.id
          assert_not_equal alias_agent.interpretation.id, alias_new_agent.interpretation.id
          unless ['type_number', 'type_regex'].include? alias_agent.nature
            assert_not_equal alias_agent.interpretation_aliasable.id, alias_new_agent.interpretation_aliasable.id
          end
        end
      end
    end
  end

  test 'Duplicate agent with regression tests' do
    create_agent_regression_check_fixtures
    agent = agents(:weather)
    @regression_weather_forecast.got = {
      'package' => intents(:weather_forecast).agent.id,
      'id' => intents(:weather_forecast).id,
      'solution' => interpretations(:weather_forecast_tomorrow).solution.to_json.to_s
    }
    assert @regression_weather_forecast.save

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert new_agent.save

    assert_equal @regression_weather_forecast.sentence, new_agent.agent_regression_checks.first.sentence
    assert_nil new_agent.agent_regression_checks.first.now
    assert_equal @regression_weather_forecast.language, new_agent.agent_regression_checks.first.language
    assert_equal @regression_weather_forecast.expected, new_agent.agent_regression_checks.first.expected
    assert_equal @regression_weather_forecast.got, new_agent.agent_regression_checks.first.got
    assert_equal @regression_weather_forecast.state, new_agent.agent_regression_checks.first.state
    assert_equal @regression_weather_forecast.position, new_agent.agent_regression_checks.first.position

    assert_equal @regression_weather_question.sentence, new_agent.agent_regression_checks.second.sentence
    assert_equal @regression_weather_question.now, new_agent.agent_regression_checks.second.now
    assert_equal @regression_weather_question.language, new_agent.agent_regression_checks.second.language
    assert_equal @regression_weather_question.expected, new_agent.agent_regression_checks.second.expected
    assert_nil new_agent.agent_regression_checks.second.got
    assert_equal @regression_weather_question.state, new_agent.agent_regression_checks.second.state
    assert_equal @regression_weather_question.position, new_agent.agent_regression_checks.second.position
  end

end
