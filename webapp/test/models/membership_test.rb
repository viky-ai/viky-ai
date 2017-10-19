require 'test_helper'

class MembershipTest < ActiveSupport::TestCase

  test "agent_id & user_id uniquiness" do
    membership = Membership.new(user: users(:admin), agent: agents(:weather))
    assert !membership.save
    assert_equal ["User has already been taken"], membership.errors.full_messages
  end

end
