require 'test_helper'

class RemoveUnconfirmedTaskTest < ActiveSupport::TestCase
  Webapp::Application.load_tasks

  test "user:create_admin" do
    Rake::Task['users:create_admin'].invoke('user_from_rake_task@voqal.ai', 'awesome password')
    assert !User.find_by_email('user_from_rake_task@voqal.ai').nil?
    assert User.find_by_email('user_from_rake_task@voqal.ai').admin?

    Rake::Task['users:create_admin'].invoke('second_user_from_rake_task@voqal.ai', '')
    assert User.find_by_email('second_user_from_rake_task@voqal.ai').nil?
  end

  test "user:set_admin" do
    u = users(:confirmed)
    assert !u.admin?
    Rake::Task['users:set_admin'].invoke('confirmed@voqal.ai')
    assert u.reload.admin?
  end

  test "user:unset_admin" do
    u = users(:admin)
    assert u.admin?
    Rake::Task['users:unset_admin'].invoke('admin@voqal.ai')
    assert !u.reload.admin?
  end

end
