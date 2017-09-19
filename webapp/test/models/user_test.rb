require 'test_helper'

class UserTest < ActiveSupport::TestCase

  test "non admin user" do
    assert !User.find_by_email('notconfirmed@voqal.ai').admin?
    assert User.find_by_email('admin@voqal.ai').admin?

    u = User.new(email: 'not-admin@voqal.ai', password: 'Hello baby')
    assert u.save
    assert !u.admin?
  end

end
