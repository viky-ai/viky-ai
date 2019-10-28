require 'test_helper'

class MembershipTest < ActiveSupport::TestCase

  test "agent_id & user_id uniquiness" do
    membership = Membership.new(user: users(:admin), agent: agents(:weather))
    assert_not membership.save
    assert_equal ["User has already been taken"], membership.errors.full_messages
  end


  test "Check integrity constraints of membership" do
    unknown_user = User.new(id: '1afbac59-0df7-44ba-bcfc-40540475ff97', name: 'Unknown user')
    exception_user = assert_raises do
      Membership.new(user: unknown_user, agent: agents(:weather)).save
    end
    assert exception_user.message.include? "PG::ForeignKeyViolation"

    unknown_agent = Agent.new(
      id: '64b95d18-83b2-4c36-8629-7fbfd84add53',
      name: 'Unknown agent',
      agentname: 'unknown_agent'
    )
    exception_agent = assert_raises do
      Membership.new(user: users(:confirmed), agent: unknown_agent).save
    end
    assert exception_agent.message.include? "PG::ForeignKeyViolation"
  end


  test "Share agent with a user" do
    user = users(:confirmed)
    agent_weather = agents(:weather)
    rights = 'edit'

    memberships_creator = MembershipsCreator.new(agent_weather, " #{user.username} , ; ", rights)
    assert memberships_creator.create
    assert_equal 1, memberships_creator.new_collaborators.size
    assert_equal user.id, memberships_creator.new_collaborators.first.id

    new_membership = Membership.where(user_id: user.id, agent_id: agent_weather.id).first
    assert_equal user.id, new_membership.user_id
    assert_equal agent_weather.id, new_membership.agent_id
    assert_equal rights, new_membership.rights
  end


  test "Error on share agent because of empty user" do
    agent_weather = agents(:weather)
    rights = 'edit'

    memberships_creator = MembershipsCreator.new(agent_weather, "", rights)
    assert_not memberships_creator.create
    assert_equal "Please enter atleast one username or email.", memberships_creator.errors.first
    assert memberships_creator.new_collaborators.empty?
  end


  test "Error on share agent because of unknown user" do
    unknown_user = User.new(id: '1afbac59-0df7-44ba-bcfc-40540475ff97', email: 'unknown@user.com')
    agent_weather = agents(:weather)
    rights = 'edit'

    memberships_creator = MembershipsCreator.new(agent_weather, unknown_user.email, rights)
    assert_not memberships_creator.create
    assert_equal "Unknown user 'unknown@user.com' given", memberships_creator.errors.first
    assert memberships_creator.new_collaborators.empty?
  end


  test "Error on share agent because of unknown agent" do
    user = users(:confirmed)
    unknown_agent = Agent.new(
      id: '64b95d18-83b2-4c36-8629-7fbfd84add53',
      name: 'Unknown agent',
      agentname: 'unknown_agent'
    )
    rights = 'edit'

    memberships_creator = MembershipsCreator.new(unknown_agent, user.username, rights)
    assert_not memberships_creator.create
    assert_equal "Agent must exist", memberships_creator.errors.first

    new_membership = Membership.where(user_id: user.id, agent_id: unknown_agent.id).first
    assert new_membership.nil?
  end


  test "Error on share agent because of unkonwn rights" do
    user = users(:confirmed)
    agent_weather = agents(:weather)
    unknown_rights = 'unknown_rights'

    memberships_creator = MembershipsCreator.new(agent_weather, user.username, unknown_rights)
    assert_not memberships_creator.create
    assert_equal "Rights is not included in the list", memberships_creator.errors.first

    new_membership = Membership.where(user_id: user.id, agent_id: agent_weather.id).first
    assert new_membership.nil?
  end
end
