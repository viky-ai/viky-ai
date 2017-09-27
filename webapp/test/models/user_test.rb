require 'test_helper'

class UserTest < ActiveSupport::TestCase

  test "non admin user" do
    assert !User.find_by_email('notconfirmed@voqal.ai').admin?
    assert User.find_by_email('admin@voqal.ai').admin?

    u = User.new(email: 'not-admin@voqal.ai', password: 'Hello baby', username: 'mrwho')
    assert u.save
    assert !u.admin?
  end


  test "username uniqueness" do
    admin = User.find_by_email('admin@voqal.ai')
    admin.username = "admin"
    assert admin.save

    confirmed = User.find_by_email('confirmed@voqal.ai')
    confirmed.username = "admin"
    assert !confirmed.save
    assert_equal ["has already been taken"], confirmed.errors.messages[:username]
  end


  test "username cleanup" do
    admin = User.find_by_email('admin@voqal.ai')
    admin.username = "Admin Yeah ' Toto"
    assert admin.save
    assert_equal "admin_yeah_toto", admin.username

    admin.username = "  "
    assert !admin.save
    assert_equal "", admin.username

    admin.username = "\" 1   2 3 ABC !?/^äù"
    assert admin.save
    assert_equal "1_2_3_abc_au", admin.username
  end


  test "search & order_by" do
    s = Backend::UserSearch.new
    expected = [
      'locked@voqal.ai',
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'invited@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)


    s = Backend::UserSearch.new(sort_by: 'email')
    expected = [
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'invited@voqal.ai',
      'locked@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)
  end


  test "search & status" do
    s = Backend::UserSearch.new
    assert_equal 5, User.search(s.options).count

    s = Backend::UserSearch.new(status: 'confirmed')
    assert_equal 3, User.search(s.options).count
    expected = [
      'locked@voqal.ai',
      'admin@voqal.ai',
      'confirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)

    s = Backend::UserSearch.new(status: 'not-confirmed')
    assert_equal 2, User.search(s.options).count
    expected = [
      'invited@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)

    s = Backend::UserSearch.new(status: 'locked')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@voqal.ai", User.search(s.options).first.email
  end


  test "search by email" do
    s = Backend::UserSearch.new
    assert_equal 5, User.search(s.options).count

    s = Backend::UserSearch.new(email: 'lock')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@voqal.ai", User.search(s.options).first.email
  end


  test "search empty" do
    s = Backend::UserSearch.new
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email')
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email', email: "  ")
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email', email: "locked")
    assert !s.empty?
  end

  test "invitation status" do
    u = users(:invited)
    travel_to (u.invitation_sent_at + User.invite_for + 1.seconds)
    assert_equal :expired, u.invitation_status
    travel_to (u.invitation_sent_at + User.invite_for - 1.seconds)
    assert_equal :valid, u.invitation_status
    travel_back
  end


  test "Users with blank username are allowed during invitation" do
    User.invite!({email: "tester_nil_1@voqal.ai"}, users(:admin))
    User.invite!({email: "tester_nil_2@voqal.ai"}, users(:admin))

    assert !User.find_by_email("tester_nil_1@voqal.ai").nil?
    assert !User.find_by_email("tester_nil_2@voqal.ai").nil?
  end


end
