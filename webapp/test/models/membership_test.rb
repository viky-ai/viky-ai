require 'test_helper'

class MembershipTest < ActiveSupport::TestCase

  test "agent_id & user_id uniquiness" do
    exception = assert_raises do
      Membership.new(user: users(:admin), agent: agents(:weather)).save
    end
    assert exception.message.include? "PG::UniqueViolation"
  end

end
