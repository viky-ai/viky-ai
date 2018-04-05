require 'test_helper'

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
end
