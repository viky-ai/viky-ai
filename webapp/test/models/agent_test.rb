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
    assert_equal 'admin', agent.users.first.username
    assert_equal ['weather','agenta'], users(:admin).agents.collect{|a| a.agentname }
  end


  test "Agent & user presence" do
    agent = Agent.new(
      name: "Agent A",
      agentname: "agenta",
      description: "Agent A decription"
    )
    assert !agent.save
    assert_equal ["Users can't be blank"], agent.errors.full_messages
  end


  test "Name & agentname validation" do
    agent = Agent.new
    assert !agent.valid?
    expected = [
      "Name can't be blank",
      "Agentname is too short (minimum is 3 characters)",
      "Agentname can't be blank",
      "Users can't be blank"
    ]
    assert_equal expected, agent.errors.full_messages

    agent = Agent.new(
      name: "",
      agentname: ""
    )
    assert !agent.valid?
    assert_equal expected, agent.errors.full_messages

    agent = Agent.new(
      name: " ",
      agentname: " "
    )
    assert !agent.valid?
    assert_equal expected, agent.errors.full_messages

    agent = Agent.new(
      name: "Agent A",
      agentname: "aa"
    )
    assert !agent.valid?
    expected = ["Agentname is too short (minimum is 3 characters)", "Users can't be blank"]
    assert_equal expected, agent.errors.full_messages
  end

end
