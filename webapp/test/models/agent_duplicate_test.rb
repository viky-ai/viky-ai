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
    new_agent = AgentDuplicator.duplicate(agent, users(:admin))

    assert new_agent.id.nil?
    assert_equal "#{agent.name} [COPY]", new_agent.name
    assert_equal agent.description, new_agent.description
    assert_equal agent.color, new_agent.color
    assert_equal "#{agent.agentname}_copy", new_agent.agentname
    assert_equal 'is_private', new_agent.visibility
    assert_not_equal agent.api_token, new_agent.api_token
    assert new_agent.owner.nil?

    assert new_agent.save
    assert_not_equal agent.id, new_agent.id
    assert_equal users(:admin).id, new_agent.owner.id
    assert_not_equal agent.created_at, new_agent.created_at
    assert_not_equal agent.updated_at, new_agent.updated_at

    another_agent = AgentDuplicator.duplicate(agent, users(:admin))
    assert_equal "#{agent.name} [COPY_1]", another_agent.name
    assert_equal "#{agent.agentname}_copy_1", another_agent.agentname
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

  test 'Duplicate agent with direct relations' do
    agent = agents(:terminator)
    parent1_agent = create_agent('Parent1 agent')
    parent2_agent = create_agent('Parent2 agent')
    assert AgentArc.create(source: agent, target: parent1_agent)
    assert AgentArc.create(source: agent, target: parent2_agent)
    child_agent = create_agent('Child agent')
    assert AgentArc.create(source: child_agent, target: agent)

    new_agent = AgentDuplicator.duplicate(agent, users(:admin))

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
end
