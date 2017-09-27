require 'test_helper'

class RemoveUnconfirmedTaskTest < ActiveSupport::TestCase
  Webapp::Application.load_tasks

  test "user:invite_admin" do
    Rake::Task['users:invite_admin'].invoke('user_from_rake_task@voqal.ai')
    assert !User.find_by_email('user_from_rake_task@voqal.ai').nil?
    assert User.find_by_email('user_from_rake_task@voqal.ai').admin?
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
