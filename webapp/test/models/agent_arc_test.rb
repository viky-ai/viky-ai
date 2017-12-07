require 'test_helper'

class AgentArcTest < ActiveSupport::TestCase

  def create_agent(name)
    agent = Agent.new(
      name: name,
      agentname: name.parameterize
    )
    agent.memberships << Membership.new(user_id: users(:admin).id, rights: "all")
    assert agent.save
    agent
  end

  test "basic agents association" do
    agent_a = create_agent("Agent A")
    agent_b = create_agent("Agent B")

    assert AgentArc.create(source: agent_a, target: agent_b)

    #   A
    #  /
    # B
    assert_equal [], agent_a.predecessors.reload.collect(&:name)
    assert_equal ["Agent B"], agent_a.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_b.predecessors.reload.collect(&:name)
    assert_equal [], agent_b.successors.reload.collect(&:name)


    agent_c = create_agent("Agent C")
    assert AgentArc.create(source: agent_a, target: agent_c)
    #   A
    #  / \
    # B   C
    assert_equal [], agent_a.predecessors.reload.collect(&:name)
    assert_equal ["Agent B", "Agent C"], agent_a.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_b.predecessors.reload.collect(&:name)
    assert_equal [], agent_b.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.reload.collect(&:name)
    assert_equal [], agent_c.successors.reload.collect(&:name)


    agent_d = create_agent("Agent D")
    assert AgentArc.create(source: agent_b, target: agent_d)
    #   A
    #  / \
    # B   C
    # |
    # D
    assert_equal [], agent_a.predecessors.reload.collect(&:name)
    assert_equal ["Agent B", "Agent C"], agent_a.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_b.predecessors.reload.collect(&:name)
    assert_equal ["Agent D"], agent_b.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.reload.collect(&:name)
    assert_equal [], agent_c.successors.reload.collect(&:name)

    assert_equal ["Agent B"], agent_d.predecessors.reload.collect(&:name)
    assert_equal [], agent_d.successors.reload.collect(&:name)


    assert AgentArc.create(source: agent_c, target: agent_d)
    #   A
    #  / \
    # B   C
    # | /
    # D
    assert_equal [], agent_a.predecessors.reload.collect(&:name)
    assert_equal ["Agent B", "Agent C"], agent_a.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_b.predecessors.reload.collect(&:name)
    assert_equal ["Agent D"], agent_b.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.reload.collect(&:name)
    assert_equal ["Agent D"], agent_c.successors.reload.collect(&:name)

    assert_equal ["Agent B", "Agent C"], agent_d.predecessors.reload.collect(&:name)
    assert_equal [], agent_d.successors.reload.collect(&:name)


    assert_equal 4, AgentArc.count
    assert Agent.find_by_name("Agent B").destroy
    #   A
    #   |
    #   C
    #   |
    #   D
    assert_equal [], agent_a.predecessors.reload.collect(&:name)
    assert_equal ["Agent C"], agent_a.successors.reload.collect(&:name)

    assert_equal [], agent_b.predecessors.reload.collect(&:name)
    assert_equal [], agent_b.successors.reload.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.reload.collect(&:name)
    assert_equal ["Agent D"], agent_c.successors.reload.collect(&:name)

    assert_equal ["Agent C"], agent_d.predecessors.reload.collect(&:name)
    assert_equal [], agent_d.successors.reload.collect(&:name)

    assert_equal 2, AgentArc.count
  end


  test 'Ensure no cycle is present' do
    #   A
    #  / \
    # B   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    AgentArc.create(source: agent_a, target: agent_b)
    AgentArc.create(source: agent_c, target: agent_a)

    arc = AgentArc.new(source: agent_b, target: agent_c)

    assert arc.invalid?
    expected = ['Cycle detected']
    assert_equal expected, arc.errors.full_messages
  end

end
