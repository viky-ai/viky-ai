require 'test_helper'

class AgentTest < ActiveSupport::TestCase

  test "basic agent creation & user association" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription",
      visibility: 'is_public'
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save

    assert_equal users(:admin).id, agent.owner_id
    assert_equal 'admin', agent.owner.username
    assert_equal ['agenta', 'terminator', 'weather'], users(:admin).agents.collect(&:agentname).sort
    assert_equal 'is_public', agent.visibility
    assert agent.is_public?
    assert !agent.is_private?
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
    assert_equal ['confirmed', 'admin'].sort, agent.users.collect(&:username).sort
  end


  test "Add collaborators and succeed" do
    agent = agents(:weather)

    assert_equal "admin", agent.owner.username
    expected = ['show_on_agent_weather', 'edit_on_agent_weather']
    assert_equal expected, agent.collaborators.collect(&:username)

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
      name: " ",
      agentname: " "
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


  test "agent color validation" do
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


  test "Search agent empty" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 1, s.options.size
    assert_equal user_id, s.options[:user_id]
  end


  test "Search agent by name" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: '800')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test "Search agent by agentname" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: 'inator')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test "Search agent by name is trimmed" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: ' inator     ')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents with membership or public' do
    user_id = users(:admin).id
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save
    s = AgentSearch.new(user_id)
    assert_equal 3, Agent.search(s.options).count
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
    assert_equal 2, agent_weather.reachable_intents.count
    assert_equal ['weather_greeting', 'weather_who'], agent_weather.reachable_intents.collect(&:intentname)

    agent_public = agents(:terminator)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    assert_equal 2, agent_weather.reachable_intents.count
    assert_equal ['weather_greeting', 'weather_who'], agent_weather.reachable_intents.collect(&:intentname)

    agent_successor = agents(:weather_confirmed)
    assert Intent.create(
      intentname: 'greeting',
      locales: ['en'],
      agent: agent_successor
    )
    assert AgentArc.create(source: agent_weather, target: agent_successor)
    assert_equal 3, agent_weather.reachable_intents.count
    assert_equal ['weather_greeting', 'weather_who', 'greeting'], agent_weather.reachable_intents.collect(&:intentname)

    agent_successor.visibility = 'is_public'
    assert agent_successor.save
    assert_equal 3, agent_weather.reachable_intents.count
    assert_equal ['weather_greeting', 'weather_who', 'greeting'], agent_weather.reachable_intents.collect(&:intentname)
  end


  test 'Test agent slug generation' do
    agent = agents(:weather)
    assert_equal 'admin/weather', agent.slug
  end
end
