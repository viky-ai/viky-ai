require 'test_helper'

class MembershipTest < ActiveSupport::TestCase

  test "agent_id & user_id uniquiness" do
    membership = Membership.new(user: users(:admin), agent: agents(:weather))
    assert !membership.save
    assert_equal ["User has already been taken"], membership.errors.full_messages
  end

  test "Check integrity constraints of membership" do
    unknown_user = User.new(id: '1afbac59-0df7-44ba-bcfc-40540475ff97', name: 'Unknown user')
    exception_user = assert_raises do
      Membership.new(user: unknown_user, agent: agents(:weather)).save
    end
    assert exception_user.message.include? "PG::ForeignKeyViolation"

    unknown_agent = Agent.new(id: '64b95d18-83b2-4c36-8629-7fbfd84add53', name: 'Unknown agent', agentname: 'unknown_agent')
    exception_agent = assert_raises do
      Membership.new(user: users(:confirmed), agent: unknown_agent).save
    end
    assert exception_agent.message.include? "PG::ForeignKeyViolation"
  end

end
