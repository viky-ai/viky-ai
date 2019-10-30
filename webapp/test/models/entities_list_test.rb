require 'test_helper'
require 'model_test_helper'

class EntitiesListTest < ActiveSupport::TestCase

  test 'Basic entities_list creation & agent association' do
    assert_equal 2, agents(:weather).entities_lists.count

    entities_list = EntitiesList.new(
      listname: 'weather_towns',
      description: 'Cities all around the world',
      visibility: 'is_private',
      position: 23,
      color: 'red',
      agent: agents(:weather),
      proximity: 'glued'
    )
    assert entities_list.save

    assert_equal 23, entities_list.position
    assert_equal 'weather_towns', entities_list.listname
    assert_equal agents(:weather).id, entities_list.agent.id
    assert_equal 3, agents(:weather).entities_lists.count
    assert EntitiesList::AVAILABLE_COLORS.one? { |color| color == entities_list.color }
    assert_equal 'is_private', entities_list.visibility
    assert entities_list.proximity_glued?
    assert_not entities_list.is_public?
    assert entities_list.is_private?
  end


  test 'listname and agent are mandatory' do
    entities_list = EntitiesList.new(description: 'Hello random citizen !')
    assert_not entities_list.save

    expected = [
      "Agent must exist",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank"
    ]
    assert_equal expected, entities_list.errors.full_messages
  end


  test 'Set position on new entities list' do
    entities_list = EntitiesList.new(
      listname: 'weather_towns',
      agent: agents(:weather)
    )
    assert entities_list.save
    assert_equal 2, entities_list.position
  end


  test 'Unique listname per agent' do
    entities_list = EntitiesList.new(listname: 'hello', description: 'Hello random citizen !', agent: agents(:weather))
    assert entities_list.save
    other_entities_list = EntitiesList.new(listname: 'hello', description: 'Another way to greet you...', agent: agents(:weather))
    assert_not other_entities_list.save

    expected = ['ID has already been taken']
    assert_equal expected, other_entities_list.errors.full_messages
  end


  test 'Check listname minimal length' do
    entities_list = EntitiesList.new(listname: 'h', description: 'Hello random citizen !', agent: agents(:weather))
    assert_not entities_list.save

    expected = ['ID is too short (minimum is 3 characters)']
    assert_equal expected, entities_list.errors.full_messages
  end


  test 'Check listname is cleaned' do
    entities_list = EntitiesList.new(listname: "H3ll-#'!o", description: 'Hello random citizen !', agent: agents(:weather))
    assert entities_list.save

    assert_equal 'h3ll-o', entities_list.listname
  end


  test 'Check entities_list slug' do
    entities_list = entities_lists(:weather_conditions)
    original_listname = entities_list.listname
    entities_list.listname = 'bonjour'
    assert entities_list.save

    assert_equal entities_list.id, EntitiesList.friendly.find(original_listname).id
  end


  test 'Do not override color at creation if already set' do
    entities_list = EntitiesList.new(
      listname: 'greeting',
      color: 'blue',
      agent: agents(:weather)
    )
    assert entities_list.save
    assert_equal 'blue', entities_list.color
  end


  test 'Test entities_list slug generation' do
    entities_list = entities_lists(:weather_conditions)
    assert_equal 'admin/weather/entities_lists/weather_conditions', entities_list.slug
  end


  test 'Entities list destroy' do
    entities_list = entities_lists(:weather_conditions)
    elist_id = entities_list.id

    assert_equal 1, EntitiesList.where(id: elist_id).count
    assert entities_list.destroy
    assert_equal 0, EntitiesList.where(id: elist_id).count
  end


  test 'Test update entities list positions' do
    agent = agents(:weather_confirmed)
    entities_list_0 = EntitiesList.create(
      listname: 'interpretation_0',
      position: 1,
      agent: agent
    )
    entities_list_1 = EntitiesList.create(
      listname: 'entities_list_1',
      position: 2,
      agent: agent
    )
    entities_list_2 = EntitiesList.create(
      listname: 'entities_list_2',
      position: 3,
      agent: agent
    )

    new_positions = [entities_list_1.id, entities_list_2.id, entities_list_0.id, '132465789']
    EntitiesList.update_positions(agent, [], new_positions)
    force_reset_model_cache([entities_list_0, entities_list_1, entities_list_2])
    assert_equal [3, 2, 1], [entities_list_1.position, entities_list_2.position, entities_list_0.position]
    assert_equal %w(is_private is_private is_private), [entities_list_1.visibility, entities_list_2.visibility, entities_list_0.visibility]
  end


  test 'Export entities_list' do
    entities_list = entities_lists(:weather_dates)
    string_io = StringIO.new
    entities_list.to_csv_in_io(string_io)
    csv = string_io.string
    expected = ["Terms,Auto solution,Solution,Case sensitive,Accent sensitive",
                "aujourd'hui:fr|tout à l'heure:fr|today:en,false,\"{\"\"date\"\": \"\"today\"\"}\",false,false",
                "tomorrow,false,\"{\"\"date\"\": \"\"tomorrow\"\"}\",false,false",
                ""].join("\n")
    assert_equal expected, csv
  end


  test 'Export uncompleted list' do
    entities_list = entities_lists(:weather_conditions)
    sun = entities(:weather_sunny)
    sun.terms = [{ term: 'sun', locale: Locales::ANY }]
    sun.solution = ''
    sun.save

    string_io = StringIO.new
    entities_list.to_csv_in_io(string_io)
    csv = string_io.string
    expected = ["Terms,Auto solution,Solution,Case sensitive,Accent sensitive",
                "sun,true,sun,false,false",
                "pluie:fr|rain:en,true,pluie,false,false",
                ''].join("\n")
    assert_equal expected, csv
  end


  test 'Move entities list to an agent' do
    weather = entities_lists(:weather_conditions)
    assert weather.move_to_agent(agents(:terminator))
    assert_equal agents(:terminator).id, weather.agent.id
    assert_equal 'is_public', weather.visibility
    assert_equal 1, weather.position
  end


  test 'Move entities list to an unknown agent' do
    weather = entities_lists(:weather_conditions)
    assert_not weather.move_to_agent(nil)
    expected = ['Agent does not exist anymore']
    assert_equal expected, weather.errors.full_messages
    assert_equal weather.agent.id, agents(:weather).id
  end


  test 'Move entities list without enough quota' do
    Feature.with_quota_enabled do
      entities_list = entities_lists(:weather_conditions)
      destination_agent = agents(:terminator)

      assert_equal 2, entities_list.entities_count
      assert_equal 10, destination_agent.owner.expressions_count

      Quota.stubs(:expressions_limit).returns(11)
      assert_not entities_list.move_to_agent(destination_agent)
      expected = ["Agent owner does not have enough quota to accept this move"]
      assert_equal expected, entities_list.errors.full_messages

      Quota.stubs(:expressions_limit).returns(12)
      assert entities_list.move_to_agent(destination_agent)
    end
  end


  test 'Get interpretations that are using the current entity (aliased_interpretations)' do
    current_entity = entities_lists(:weather_dates)
    expected = [interpretations(:weather_forecast)]
    actual = current_entity.aliased_interpretations
    assert_equal expected, actual
  end


  test 'No interpretations that are using the current entity (aliased_interpretations)' do
    current_entity = entities_lists(:terminator_targets)
    expected = []
    actual = current_entity.aliased_interpretations
    assert_equal expected, actual
  end


  test 'List entities by batch orderer by position' do
    entities_list = entities_lists(:terminator_targets)
    targets = (0...6).map do |target_id|
      Entity.create!(
        auto_solution_enabled: true,
        solution: "target_#{target_id}",
        terms: [
          { term: "target_#{target_id}", locale: '*' }
        ],
        position: target_id,
        entities_list: entities_list
      )
    end
    entities_list.entities = targets
    assert entities_list.save

    entities_list.entities_in_ordered_batchs(4).each_with_index do |params, index|
      batch = params[0]
      max = params[1]
      min = params[2]
      if index.zero?
        offset = 2
        assert_equal 5, max
        assert_equal 2, min
      else
        offset = -2
        assert_equal 1, max
        assert_equal 0, min
      end
      assert_equal 3 + offset, batch[0].position
      assert_equal 2 + offset, batch[1].position
      assert_equal 1 + offset, batch[2].position if index.zero?
      assert_equal 0 + offset, batch[3].position if index.zero?
    end

    entities_list.entities = []
    assert entities_list.save
    entities_list.entities_in_ordered_batchs(4).each do |batch, max_position, min_position|
      assert_empty batch
      assert_equal 0, max_position
      assert_equal 0, min_position
    end
  end


  test 'List entities in order when only one entity is in the list' do
    entities_list = entities_lists(:terminator_targets)
    single_entity = Entity.create!(
      auto_solution_enabled: true,
      solution: 'target_0',
      terms: [
        { term: 'target_0', locale: '*' }
      ],
      position: 0,
      entities_list: entities_list
    )
    entities_list.entities = [single_entity]
    assert entities_list.save

    result = 0
    entities_list.entities_in_ordered_batchs(4).each do |batch, max_position, min_position|
      result += 1
      assert_equal 0, max_position
      assert_equal 0, min_position
      assert_equal 'target_0', batch.first.solution
    end
    assert_equal 1, result
  end


  test 'Adding an entity trigger an NLP sync' do
    entities_list = entities_lists(:terminator_targets)
    entities_list.agent.expects(:sync_nlp).once

    entity = Entity.new(
      auto_solution_enabled: true,
      solution: 'target_0',
      terms: [
        { term: 'target_0', locale: '*' }
      ],
      position: 0,
      entities_list: entities_list
    )
    assert entity.save
  end

end
