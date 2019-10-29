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
    assert new_agent.persisted?

    assert 0, new_agent.errors.size
    assert_equal "#{agent.name} [COPY]", new_agent.name
    assert_equal agent.description, new_agent.description
    assert_equal agent.color, new_agent.color
    assert_equal "#{agent.agentname}-copy", new_agent.agentname
    assert_equal 'is_private', new_agent.visibility
    assert_equal agent.locales, new_agent.locales
    assert_not_equal agent.api_token, new_agent.api_token

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
    assert new_agent.persisted?
    assert_equal current_user.id, new_agent.owner.id

    another_agent = AgentDuplicator.new(agent, current_user).duplicate
    assert another_agent.persisted?
  end


  test 'Duplicate keeps track of source agent' do
    agent = agents(:terminator)
    current_user = users(:show_on_agent_weather)

    new_agent = AgentDuplicator.new(agent, current_user).duplicate
    assert new_agent.persisted?

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

    assert new_agent.persisted?
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
    assert FormulationAlias.create(
      position_start: 31,
      position_end: 32,
      aliasname: 'number',
      formulation: formulations(:weather_forecast_tomorrow),
      nature: 'type_number'
    )
    assert FormulationAlias.create(
      position_start: 35,
      position_end: 40,
      aliasname: 'regex',
      formulation: formulations(:weather_forecast_tomorrow),
      nature: 'type_regex',
      reg_exp: 'A'
    )

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert 0, new_agent.errors.size

    assert_equal agent.entities_lists.size, new_agent.entities_lists.size
    entities_list = agent.entities_lists.zip(new_agent.entities_lists)
    entities_list.each do |elist_agent, elist_new_agent|
      assert_equal elist_agent.listname, elist_new_agent.listname
      assert_equal elist_agent.visibility, elist_new_agent.visibility
      assert_not_equal elist_agent.id, elist_new_agent.id
      entities = elist_agent.entities.order(:position).zip(elist_new_agent.entities.order(:position))
      entities.each do |entity_agent, entity_new_agent|
        assert_equal entity_agent.terms, entity_new_agent.terms
        assert entity_agent.solution == entity_new_agent.solution
        assert_equal entity_agent.auto_solution_enabled, entity_new_agent.auto_solution_enabled
        assert_equal entity_agent.position, entity_new_agent.position
        assert_equal entity_agent.searchable_terms, entity_new_agent.searchable_terms
        assert_not_equal entity_agent.id, entity_new_agent.id
        assert_not_equal entity_agent.entities_list.id, entity_new_agent.entities_list.id

      end

      assert_equal elist_agent.formulation_aliases.size, elist_new_agent.formulation_aliases.size
      aliases = elist_agent.formulation_aliases.zip(elist_new_agent.formulation_aliases)
      aliases.each do |alias_agent, alias_new_agent|
        assert_equal alias_agent.aliasname, alias_new_agent.aliasname
        assert_equal alias_agent.position_start, alias_new_agent.position_start
        assert_equal alias_agent.position_end, alias_new_agent.position_end
        assert_equal alias_agent.nature, alias_new_agent.nature
        assert_equal alias_agent.is_list, alias_new_agent.is_list
        assert_equal alias_agent.any_enabled, alias_new_agent.any_enabled
        assert_not_equal alias_agent.id, alias_new_agent.id
        assert_not_equal alias_agent.formulation.id, alias_new_agent.formulation.id
        assert_not_equal alias_agent.formulation_aliasable.id, alias_new_agent.formulation_aliasable.id
      end
    end


    assert_equal agent.intents.size, new_agent.intents.size
    intents = agent.intents.zip(new_agent.intents)
    intents.each do |intent_agent, intent_new_agent|
      assert_equal intent_agent.intentname, intent_new_agent.intentname
      assert_equal intent_agent.visibility, intent_new_agent.visibility
      assert_not_equal intent_agent.id, intent_new_agent.id

      assert_equal intent_agent.formulations.size, intent_new_agent.formulations.size
      formulations = intent_agent.formulations.zip(intent_new_agent.formulations)
      formulations.each do |inter_agent, inter_new_agent|
        assert_equal inter_agent.expression, inter_new_agent.expression
        assert_equal inter_agent.locale, inter_new_agent.locale
        assert_equal inter_agent.keep_order, inter_new_agent.keep_order
        assert_equal inter_agent.proximity.to_s, inter_new_agent.proximity.to_s
        assert_equal inter_agent.auto_solution_enabled, inter_new_agent.auto_solution_enabled
        assert inter_agent.solution == inter_new_agent.solution
        assert_not_equal inter_agent.id, inter_new_agent.id
        assert_not_equal inter_agent.intent.id, inter_new_agent.intent.id

        assert_equal inter_agent.formulation_aliases.size, inter_new_agent.formulation_aliases.size
        aliases = inter_agent.formulation_aliases.zip(inter_new_agent.formulation_aliases)
        aliases.each do |alias_agent, alias_new_agent|
          assert_equal alias_agent.aliasname, alias_new_agent.aliasname
          assert_equal alias_agent.position_start, alias_new_agent.position_start
          assert_equal alias_agent.position_end, alias_new_agent.position_end
          assert_equal alias_agent.nature, alias_new_agent.nature
          assert_equal alias_agent.is_list, alias_new_agent.is_list
          assert_equal alias_agent.any_enabled, alias_new_agent.any_enabled
          assert_not_equal alias_agent.id, alias_new_agent.id
          assert_not_equal alias_agent.formulation.id, alias_new_agent.formulation.id
          unless ['type_number', 'type_regex'].include? alias_agent.nature
            assert_not_equal alias_agent.formulation_aliasable.id, alias_new_agent.formulation_aliasable.id
          end
        end
      end
    end
  end


  test 'Duplicate agent with regression tests' do
    create_agent_regression_check_fixtures
    agent = agents(:weather)
    @regression_weather_forecast.expected = {
      root_type: 'intent',
      package: agents(:weather_confirmed).id,
      id: intents(:weather_confirmed_question).id,
      solution: ''
    }
    @regression_weather_forecast.got = {
      'package' => intents(:weather_forecast).agent.id,
      'id' => intents(:weather_forecast).id,
      'solution' => formulations(:weather_forecast_tomorrow).solution.to_json.to_s
    }
    assert @regression_weather_forecast.save

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert new_agent.persisted?

    duplicated_tests = new_agent.agent_regression_checks.order(:position)
    assert_equal 3, duplicated_tests.size
    assert_equal @regression_weather_forecast.sentence, duplicated_tests.first.sentence
    assert_nil duplicated_tests.first.now
    assert_equal @regression_weather_forecast.language, duplicated_tests.first.language
    assert_equal @regression_weather_forecast.expected, duplicated_tests.first.expected
    assert_equal @regression_weather_forecast.got, duplicated_tests.first.got
    assert_equal @regression_weather_forecast.state, duplicated_tests.first.state
    assert_equal @regression_weather_forecast.position, duplicated_tests.first.position

    assert_equal @regression_weather_question.sentence, duplicated_tests.second.sentence
    assert_equal @regression_weather_question.now, duplicated_tests.second.now
    assert_equal @regression_weather_question.language, duplicated_tests.second.language
    new_expected = {
      'package' => new_agent.id,
      'id' => new_agent.intents.where(intentname: 'weather_question').first.id,
      'solution' => formulations(:weather_question_like).solution.to_json.to_s,
      'root_type' => 'intent'
    }
    assert_equal new_expected, duplicated_tests.second.expected
    assert_nil duplicated_tests.second.got
    assert_equal @regression_weather_question.state, duplicated_tests.second.state
    assert_equal @regression_weather_question.position, duplicated_tests.second.position

    assert_equal @regression_weather_condition.sentence, duplicated_tests.third.sentence
    assert_equal @regression_weather_condition.now, duplicated_tests.third.now
    assert_equal @regression_weather_condition.language, duplicated_tests.third.language
    new_expected = {
      'package' => new_agent.id,
      'id' => new_agent.entities_lists.where(listname: 'weather_conditions').first.id,
      'solution' => entities(:weather_sunny).solution.to_json.to_s,
      'root_type' => 'entities_list'
    }
    assert_equal new_expected, duplicated_tests.third.expected
    assert_nil duplicated_tests.third.got
    assert_equal @regression_weather_condition.state, duplicated_tests.third.state
    assert_equal @regression_weather_condition.position, duplicated_tests.third.position
  end


  test 'The order of agent associations with property position should be maintained' do
    agent = agents(:weather)
    assert agent.entities_lists.destroy_all
    assert agent.intents.destroy(intents(:weather_question))

    intent = intents(:weather_forecast)
    assert intent.formulations.destroy_all

    formulation_0 = Formulation.create(
      expression: 'formulation_0',
      locale: Locales::ANY,
      position: 0,
      intent: intent
    )
    formulation_1 = Formulation.create(
      expression: 'formulation_1',
      locale: Locales::ANY,
      position: 1,
      intent: intent
    )
    formulation_2 = Formulation.create(
      expression: 'formulation_2',
      locale: Locales::ANY,
      position: 2,
      intent: intent
    )

    new_positions = [formulation_2.id, formulation_0.id, formulation_1.id]
    Formulation.update_positions(intent, new_positions)
    force_reset_model_cache([formulation_0, formulation_1, formulation_2])
    assert_equal [2, 1, 0], [formulation_2.position, formulation_0.position, formulation_1.position]

    new_agent = AgentDuplicator.new(agent, users(:admin)).duplicate
    assert new_agent.persisted?

    assert_equal agent.intents.size, new_agent.intents.size
    intents = agent.intents.zip(new_agent.intents)
    intents.each do |intent_agent, intent_new_agent|
      assert_not_equal intent_agent.id, intent_new_agent.id
      assert_equal intent_agent.intentname, intent_new_agent.intentname
      assert_equal intent_agent.position, intent_new_agent.position

      assert_equal intent_agent.formulations.size, intent_new_agent.formulations.size
      formulations = intent_agent.formulations.order(:position).zip(intent_new_agent.formulations.order(:position))
      formulations.each do |inter_agent, inter_new_agent|
        assert_equal inter_agent.expression, inter_new_agent.expression
        assert_equal inter_agent.position, inter_new_agent.position
        assert_not_equal inter_agent.id, inter_new_agent.id
        assert_not_equal inter_agent.intent.id, inter_new_agent.intent.id
      end
    end
  end


  test 'Should not be able to duplicate an agent according to quota' do
    Feature.with_quota_enabled do
      weather_agent = agents(:weather)
      admin_user = users(:admin)

      assert_equal 10, admin_user.expressions_count
      assert_equal 7, weather_agent.expressions_count

      duplicator = AgentDuplicator.new(weather_agent, admin_user)

      Quota.stubs(:expressions_limit).returns(10)
      assert_not duplicator.respects_quota?

      Quota.stubs(:expressions_limit).returns(15)
      assert_not duplicator.respects_quota?

      Quota.stubs(:expressions_limit).returns(17)
      assert duplicator.respects_quota?

      Quota.stubs(:expressions_limit).returns(20)
      assert duplicator.respects_quota?
    end
  end

end
