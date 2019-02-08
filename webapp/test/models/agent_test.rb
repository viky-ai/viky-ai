require 'test_helper'
require 'model_test_helper'

class AgentTest < ActiveSupport::TestCase

  test "Basic agent creation & user association" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription",
      visibility: 'is_public',
      nlp_updated_at: '2019-01-21 10:07:53.484942',
      source_agent: {
        id: agents(:terminator).id,
        slug: agents(:terminator).slug,
        date: '2017-01-02 12:34:56'
      }
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save
    assert_equal users(:admin).id, agent.owner_id
    assert_equal 'admin', agent.owner.username
    assert_equal ['agenta', 'terminator', 'weather'], users(:admin).agents.collect(&:agentname).sort
    assert_equal 'is_public', agent.visibility
    assert agent.is_public?
    assert !agent.is_private?
    assert agents(:terminator).id, agent.source_agent[:id]
    assert agents(:terminator).slug, agent.source_agent[:slug]
    assert '2017-01-02 12:34:56', agent.source_agent[:date]
  end


  test "Agent & user presence" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    assert !agent.save
    expected = ["Owner can't be blank", "Users list does not includes agent owner"]
    assert_equal expected, agent.errors.full_messages
  end


  test "Add multiple users and owner_id does not change" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save
    assert_equal users(:admin).id, agent.owner_id
    assert_equal users(:admin).id, agent.owner.id

    agent.users << users(:confirmed)
    assert agent.save
    assert_equal users(:admin).id, agent.owner_id
    assert_equal users(:admin).id, agent.owner.id
    assert_equal ['confirmed'], agent.collaborators.collect(&:username)
    assert agent.collaborator?(users(:confirmed))
    assert_equal ['confirmed', 'admin'].sort, agent.users.collect(&:username).sort
  end


  test "Add collaborators and succeed" do
    agent = agents(:weather)
    assert_equal "admin", agent.owner.username
    expected = ['show_on_agent_weather', 'edit_on_agent_weather']
    assert_equal expected, agent.collaborators.collect(&:username)
    assert agent.collaborator?(users(:show_on_agent_weather))
    assert agent.collaborator?(users(:edit_on_agent_weather))

    agent.memberships << Membership.new(user_id: users(:confirmed).id, rights: "edit")
    agent.memberships << Membership.new(user_id: users(:locked).id, rights: "show")
    assert agent.save
    expected = ['confirmed', 'locked', 'show_on_agent_weather', 'edit_on_agent_weather'].sort
    assert_equal expected, agent.collaborators.collect(&:username).sort
  end


  test "Add owner as collaborators" do
    agent = agents(:weather)
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "edit")
    assert !agent.save
    assert_equal ['Memberships is invalid'], agent.errors.full_messages
  end


  test "Add same collaborator" do
    agent = agents(:weather)
    agent.memberships << Membership.new(user_id: users(:confirmed).id, rights: "edit")
    agent.memberships << Membership.new(user_id: users(:confirmed).id, rights: "show")
    assert !agent.save
    assert_equal ['Memberships is invalid'], agent.errors.full_messages
  end


  test "Add collaborator with bad rights" do
    agent = agents(:weather)
    agent.memberships << Membership.new(user_id: users(:confirmed).id, rights: "missing rights")
    assert !agent.save
    assert_equal ['Memberships is invalid'], agent.errors.full_messages
  end


  test "Remove users from agent & ensure owner stay present" do
    agent = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save

    agent.users = []
    assert !agent.valid?
    expected = ["Users list does not includes agent owner"]
    assert_equal expected, agent.errors.full_messages

    agent.users << users(:confirmed)
    assert !agent.save
    expected = ["Users list does not includes agent owner"]
    assert_equal expected, agent.errors.full_messages
  end


  test "Name & agentname nil validation" do
    agent = Agent.new
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert !agent.valid?
    expected = [
      "Name can't be blank",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank"
    ]
    assert_equal expected, agent.errors.full_messages
  end


  test "Name & agentname blank validation" do
    agent = Agent.new(
      name: "",
      agentname: ""
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert !agent.valid?
    expected = [
      "Name can't be blank",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank"
    ]
    assert_equal expected, agent.errors.full_messages

    agent = Agent.new(
      name: ' ',
      agentname: ' '
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert !agent.valid?
    assert_equal expected, agent.errors.full_messages
  end


  test "agentname length validation" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "aa"
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert !agent.valid?
    expected = ["ID is too short (minimum is 3 characters)"]
    assert_equal expected, agent.errors.full_messages
  end


  test "agentname uniqueness validation" do
    agent_1 = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent_1.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent_1.valid?
    assert agent_1.save

    agent_2 = Agent.new(
      name: "Agent 2",
      agentname: "aaa"
    )
    agent_2.memberships << Membership.new(user_id: users(:admin).id, rights: "all")

    assert !agent_2.valid?
    expected = ["ID has already been taken"]
    assert_equal expected, agent_2.errors.full_messages
  end


  test "Agent color validation" do
    agent = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.valid?
    assert agent.save

    agent.color = "missing value"
    assert agent.invalid?
    expected = ["Color is not included in the list"]
    assert_equal expected, agent.errors.full_messages

    agent.color = "red"
    assert agent.valid?
  end


  test "Test agent slug" do
    agent = users(:admin).agents.friendly.find("weather")
    assert_equal "My awesome weather bot", agent.name

    agent.agentname = 'new-weather'
    assert agent.save
    agent = users(:admin).agents.friendly.find("weather")
    assert_equal "My awesome weather bot", agent.name
    agent = users(:admin).agents.friendly.find("new-weather")
    assert_equal "My awesome weather bot", agent.name

    agent.agentname = 'new-new-weather'
    assert agent.save
    agent = users(:admin).agents.friendly.find("weather")
    assert_equal "My awesome weather bot", agent.name
    agent = users(:admin).agents.friendly.find("new-weather")
    assert_equal "My awesome weather bot", agent.name
    agent = users(:admin).agents.friendly.find("new-new-weather")
    assert_equal "My awesome weather bot", agent.name
  end


  test "Clean agentname" do
    agent = Agent.new(
      name: "Agent 2",
      agentname: "aaa?# b"
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save
    assert_equal "aaa-b", agent.agentname
  end


  test "Transfer agent ownership and keep edit rights for previous owner" do
    user_admin = users(:admin)
    user_confirmed = users(:confirmed)
    terminator_agent = agents(:terminator)

    assert_equal user_admin.id, terminator_agent.owner_id
    assert terminator_agent.users.one? { |user| user.id == user_admin.id }
    assert terminator_agent.users.none? { |user| user.id == user_confirmed.id }

    result = terminator_agent.transfer_ownership_to(user_confirmed.id)
    assert result[:success]

    assert_equal user_confirmed.id, terminator_agent.owner_id
    assert Membership.where(user_id: user_admin.id, agent_id: terminator_agent.id, rights: 'edit').one?
    assert terminator_agent.users.one? { |user| user.id == user_confirmed.id }
  end


  test "Transfer agent ownership whereas another agent exists with this agentname" do
    user_confirmed = users(:confirmed)
    weather_agent = agents(:weather)
    assert_equal 0, (user_confirmed.agents.count { |agent| agent.name == "My awesome weather bot" })

    result = weather_agent.transfer_ownership_to(user_confirmed.id)
    assert !result[:success]
    expected = ["This user already have an agent with this ID"]
    assert_equal expected, result[:errors]

    assert weather_agent.save

    assert_equal 0, (user_confirmed.agents.count { |agent| agent.name == "My awesome weather bot" })
  end


  test "Transfer agent ownership whereas new owner doesn't exit" do
    new_owner = User.new(email: 'not-admin@viky.ai', password: 'Hello baby', username: 'mrwho')
    weather_agent = agents(:terminator)

    result = weather_agent.transfer_ownership_to(new_owner.id)
    assert !result[:success]
    expected = ["Please select a valid new owner"]
    assert_equal expected, result[:errors]
  end


  test "Transfer agent ownership to collaborator who already have a membership" do
    new_owner = users(:show_on_agent_weather)
    weather_agent = agents(:weather)

    result = weather_agent.transfer_ownership_to(new_owner.id)
    assert result[:success]
  end


  test "A new agent always has a token" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    Membership.new(user: users(:admin), agent: agent).save

    agent.save
    assert !agent.api_token.nil?
  end


  test "A token is always required" do
    agent = agents(:terminator)

    agent.api_token = nil
    agent.save

    expected = [
      "can't be blank",
      "is too short (minimum is 32 characters)"
    ]
    assert_equal expected, agent.errors.messages[:api_token]
  end


  test "Api token is unique" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription",
      api_token: agents(:terminator).api_token
    )
    Membership.new(user: users(:admin), agent: agent).save

    agent.save
    assert ["has already been taken"], agent.errors.messages[:api_token]
  end


  test "Destroy validation when collaborators are presents" do
    agent = agents(:weather)
    assert_equal 5, Membership.all.count
    assert !agent.destroy
    expected = ["You must remove all collaborators before delete an agent"]
    assert_equal expected, agent.errors.full_messages
    assert_equal 5, Membership.all.count

    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end
    assert_equal 3, Membership.all.count
    assert agent.destroy
    assert_equal 2, Membership.all.count
  end


  test 'Delete agent with intents' do
    agent = agents(:weather)
    agent_id = agent.id
    assert_equal 2, Intent.where(agent_id: agent_id).count
    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end
    assert agent.destroy
    assert_equal 0, Intent.where(agent_id: agent_id).count
    assert_equal 0, agent.intents.count
  end


  test 'List reachable intents for agent' do
    agent_weather = agents(:weather)
    current_intent = Intent.create(intentname: 'current_intent', locales: ['en'], agent: agent_weather,)
    assert_equal 2, agent_weather.reachable_intents(current_intent).count
    assert_equal ['weather_forecast', 'weather_question'], agent_weather.reachable_intents(current_intent).collect(&:intentname)

    agent_successor = agents(:weather_confirmed)
    assert Intent.create(
      intentname: 'greeting',
      locales: ['en'],
      agent: agent_successor
    )
    assert AgentArc.create(source: agent_weather, target: agent_successor)
    force_reset_model_cache(agent_weather)
    assert_equal 3, agent_weather.reload.reachable_intents(intents(:weather_question)).count
    assert_equal ['current_intent', 'weather_forecast', 'greeting'], agent_weather.reachable_intents(intents(:weather_question)).collect(&:intentname)
  end


  test 'List reachable public/private intents for agent' do
    agent_weather = agents(:weather)
    current_intent = Intent.create(intentname: 'current_intent', locales: ['en'], agent: agent_weather,)
    intent_greetings = intents(:weather_forecast)
    intent_greetings.visibility = 'is_public'
    assert intent_greetings.save
    intent_who = intents(:weather_question)
    intent_who.visibility = 'is_private'
    assert intent_who.save

    agent_successor = agents(:weather_confirmed)
    assert Intent.create(
      intentname: 'greeting_public',
      locales: ['en'],
      agent: agent_successor,
      visibility: 'is_public'
    )
    assert Intent.create(
      intentname: 'greeting_private',
      locales: ['en'],
      agent: agent_successor,
      visibility: 'is_private'
    )
    assert AgentArc.create(source: agent_weather, target: agent_successor)

    force_reset_model_cache(agent_weather)
    assert_equal 3, agent_weather.reload.reachable_intents(current_intent).count
    assert_equal ['weather_forecast', 'weather_question', 'greeting_public'], agent_weather.reachable_intents(current_intent).collect(&:intentname)
  end


  test 'List reachable entities_list for agent' do
    agent_weather = agents(:weather)
    assert_equal 2, agent_weather.reachable_entities_lists.count
    assert_equal ['weather_conditions', 'weather_dates'], agent_weather.reachable_entities_lists.collect(&:listname)

    agent_successor = agents(:weather_confirmed)
    assert EntitiesList.create(
      listname: 'locations',
      agent: agent_successor
    )
    assert AgentArc.create(source: agent_weather, target: agent_successor)
    force_reset_model_cache(agent_weather)
    assert_equal 3, agent_weather.reachable_entities_lists.count
    assert_equal ['weather_conditions', 'weather_dates', 'locations'], agent_weather.reachable_entities_lists.collect(&:listname)
  end


  test 'List reachable public/private entities_list for agent' do
    agent_weather = agents(:weather)
    elist_conditions = entities_lists(:weather_conditions)
    elist_conditions.visibility = EntitiesList.visibilities[:is_public]
    assert elist_conditions.save
    elist_dates = entities_lists(:weather_dates)
    elist_dates.visibility = EntitiesList.visibilities[:is_private]
    assert elist_dates.save

    agent_successor = agents(:weather_confirmed)
    assert EntitiesList.create(
      listname: 'greeting_public',
      agent: agent_successor,
      visibility: EntitiesList.visibilities[:is_public]
    )
    assert EntitiesList.create(
      listname: 'greeting_private',
      agent: agent_successor,
      visibility: EntitiesList.visibilities[:is_private]
    )
    assert AgentArc.create(source: agent_weather, target: agent_successor)

    force_reset_model_cache(agent_weather)
    assert_equal 3, agent_weather.reachable_entities_lists.count
    assert_equal ['weather_conditions', 'weather_dates', 'greeting_public'], agent_weather.reachable_entities_lists.collect(&:listname)
  end


  test 'List available destinations' do
    current_user = users(:admin)

    weather_confirmed = agents(:weather_confirmed)
    weather_confirmed.memberships << Membership.new(user: current_user, rights: 'edit')
    assert weather_confirmed.save

    other_agent_with_edit = Agent.create(
      name: 'Other_agent_with_edit',
      agentname: 'other_agent_with_edit'.parameterize,
      memberships: [
        Membership.new(user: users(:confirmed), rights: 'all'),
        Membership.new(user: current_user, rights: 'edit')
      ]
    )
    assert other_agent_with_edit.save

    other_agent_without_edit = Agent.create(
      name: 'Other_agent_without_edit',
      agentname: 'other_agent_without_edit'.parameterize,
      memberships: [
        Membership.new(user: users(:confirmed), rights: 'all'),
      ]
    )
    assert other_agent_without_edit.save

    search = AgentSelectSearch.new(current_user)
    destinations = weather_confirmed.available_destinations(search.options).order(name: :asc)
    expected = [
      'admin/weather',
      'confirmed/other_agent_with_edit',
      'admin/terminator',
    ]
    assert_equal expected, destinations.collect(&:slug)

    filtered_search = AgentSelectSearch.new(current_user, query: 'term')
    destinations = weather_confirmed.available_destinations(filtered_search.options).order(name: :asc)
    expected = [
      'admin/terminator',
    ]
    assert_equal expected, destinations.collect(&:slug)

    assert FavoriteAgent.create(user: current_user, agent: other_agent_with_edit)
    filtered_search = AgentSelectSearch.new(current_user, filter_owner: 'favorites')
    destinations = weather_confirmed.available_destinations(filtered_search.options).order(name: :asc)
    expected = [
      'confirmed/other_agent_with_edit',
    ]
    assert_equal expected, destinations.collect(&:slug)
  end


  test 'Test agent slug generation' do
    agent = agents(:weather)
    assert_equal 'admin/weather', agent.slug
  end


  test 'Test agent regression any failed' do
    create_agent_regression_check_fixtures

    agent = agents(:weather)
    assert agent.any_tests_failed?

    @regression_weather_question.state = 'success'
    @regression_weather_question.save

    assert_not agent.any_tests_failed?
  end


  test 'Test a sentence is already tested' do
    create_agent_regression_check_fixtures

    agent = agents(:weather)
    assert agent.sentence_tested?('Quel temps fera-t-il demain ?', '*', '2019-01-21 12:00:00')
    assert agent.sentence_tested?('quel temps fera-t-il demain ?', '*', '2019-01-21 12:00:00')
    assert agent.sentence_tested?(' Quel temps fera-t-il demain ?   ', '*', '2019-01-21 12:00:00')
    assert_not agent.sentence_tested?('random input : qlsjlqsjdflqsd', '*', '2019-01-21 12:00:00')
    assert_not agent.sentence_tested?('Quel temps fera-t-il demain ?', 'fr', '2019-01-21 12:00:00')
    assert_not agent.sentence_tested?('Quel temps fera-t-il demain ?', '*', '2019-12-12 12:12:12')
  end


  test 'Test find a regression check from a sentence' do
    create_agent_regression_check_fixtures

    agent = agents(:weather)
    assert @regression_weather_forecast.id, agent.regression_check_for('Quel temps fera-t-il demain ?', '*', '2019-01-21 12:00:00').id
    assert @regression_weather_forecast.id, agent.regression_check_for('quel temps fera-t-il demain ?', '*', '2019-01-21 12:00:00').id
    assert @regression_weather_forecast.id, agent.regression_check_for(' Quel temps fera-t-il demain ?   ', '*', '2019-01-21 12:00:00').id
    assert_not agent.regression_check_for('random input : qlsjlqsjdflqsd', '*', '2019-01-21 12:00:00')
    assert_not agent.regression_check_for('Quel temps fera-t-il demain ?', 'fr', '2019-01-21 12:00:00')
    assert_not agent.regression_check_for('Quel temps fera-t-il demain ?', '*', '2019-12-12 12:12:12')
  end


  test 'Reset nlp updated at when the agent is updated' do
    agent = agents(:weather)
    assert_nil agent.nlp_updated_at
    assert_not agent.synced_with_nlp?

    agent.updated_at = '2018-01-01 01:01:01.000000'
    agent.nlp_updated_at = '2000-01-01 01:01:01.000000'
    assert_not agent.synced_with_nlp?

    agent.updated_at = '2018-01-01 01:01:01.000000'
    agent.nlp_updated_at = '2020-01-01 01:01:01.000000'
    assert agent.synced_with_nlp?

    agent.updated_at = '2018-01-01 01:01:01.000000'
    agent.nlp_updated_at = '2018-01-01 01:01:01.000000'
    assert agent.synced_with_nlp?
  end
end
