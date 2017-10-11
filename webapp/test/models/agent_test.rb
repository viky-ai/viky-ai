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
    assert_equal ['weather', 'terminator', 'agenta'], users(:admin).agents.collect(&:agentname)
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
      "Agentname is too short (minimum is 3 characters)",
      "Agentname can't be blank"
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
      "Agentname is too short (minimum is 3 characters)",
      "Agentname can't be blank"
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
    expected = ["Agentname is too short (minimum is 3 characters)"]
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
    expected = ["Agentname has already been taken"]
    assert_equal expected, agent_2.errors.full_messages
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

end
