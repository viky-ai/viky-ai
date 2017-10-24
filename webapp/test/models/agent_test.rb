require 'test_helper'

class AgentTest < ActiveSupport::TestCase

  test "basic agent creation & user association" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    agent.users << users(:admin)

    assert agent.save
    assert_equal users(:admin).id, agent.owner_id
    assert_equal 'admin', agent.users.first.username
    assert_equal ['agenta', 'terminator', 'weather'], users(:admin).agents.collect(&:agentname).sort
  end


  test "Agent & user presence" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    assert !agent.save
    expected = ["Users can't be blank", "Owner can't be blank"]
    assert_equal expected, agent.errors.full_messages
  end


  test "Add multiple users and owner_id does not change" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    agent.users << users(:admin)

    assert agent.save
    assert_equal users(:admin).id, agent.owner_id

    agent.users << users(:confirmed)
    assert agent.save
    assert_equal users(:admin).id, agent.owner_id
    assert_equal ['admin', 'confirmed'], agent.users.collect(&:username)
  end


  test "Remove users from agent & ensure owner stay present" do
    agent = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent.users << users(:admin)
    assert agent.save

    agent.users = []
    assert !agent.valid?
    expected = ["Users can't be blank"]
    assert_equal expected, agent.errors.full_messages

    agent.users << users(:confirmed)
    assert !agent.save
    expected = ["Users list does not includes agent owner"]
    assert_equal expected, agent.errors.full_messages
  end


  test "Name & agentname nil validation" do
    agent = Agent.new
    agent.users << users(:admin)
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
    agent.users << users(:admin)
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
    agent.users << users(:admin)
    assert !agent.valid?
    assert_equal expected, agent.errors.full_messages
  end


  test "agentname length validation" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "aa"
    )
    agent.users << users(:admin)
    assert !agent.valid?
    expected = ["ID is too short (minimum is 3 characters)"]
    assert_equal expected, agent.errors.full_messages
  end


  test "agentname uniqueness validation" do
    agent_1 = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent_1.users << users(:admin)
    assert agent_1.valid?
    assert agent_1.save

    agent_2 = Agent.new(
      name: "Agent 2",
      agentname: "aaa"
    )
    agent_2.users << users(:admin)

    assert !agent_2.valid?
    expected = ["ID has already been taken"]
    assert_equal expected, agent_2.errors.full_messages
  end


  test "agent color validation" do
    agent = Agent.new(
      name: "Agent 1",
      agentname: "aaa"
    )
    agent.users << users(:admin)
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
    agent.users << users(:admin)
    assert agent.save
    assert_equal "aaa-b", agent.agentname
  end


  test "Transfer agent ownership" do
    user_admin = users(:admin)
    user_confirmed = users(:confirmed)
    terminator_agent = agents(:terminator)

    assert_equal user_admin.id, terminator_agent.owner_id
    assert terminator_agent.users.one? do |user|
      user.id == user_admin.id
    end
    assert 0, terminator_agent.users.count do |user|
      user.id == user_confirmed.id
    end
    result = terminator_agent.transfer_ownership_to(user_confirmed.username)
    assert result[:success]
    assert_equal user_confirmed.id, terminator_agent.owner_id
    assert terminator_agent.users.one? do |user|
      user.id == user_confirmed.id
    end
    assert 0, terminator_agent.users.count do |user|
      user.id == user_admin.id
    end
  end


  test "Transfer agent ownership whereas another agent exists with this agentname" do
    user_admin = users(:admin)
    user_confirmed = users(:confirmed)
    weather_agent = agents(:weather)
    assert_equal 0, user_confirmed.agents.count { |agent| agent.name == "My awesome weather bot" }

    result = weather_agent.transfer_ownership_to(user_confirmed.username)
    assert !result[:success]
    expected = ["This user already have an agent with this ID"]
    assert_equal expected, result[:errors]

    assert weather_agent.save

    assert_equal 0, user_confirmed.agents.count { |agent| agent.name == "My awesome weather bot" }
  end


  test "Transfer agent ownership whereas new owner doesn't exit" do
    user_admin = users(:admin)
    new_owner = User.new(email: 'not-admin@voqal.ai', password: 'Hello baby', username: 'mrwho')
    weather_agent = agents(:terminator)

    result = weather_agent.transfer_ownership_to(new_owner.username)
    assert !result[:success]
    expected = ["Please select a valid new owner"]
    assert_equal expected, result[:errors]
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

end
