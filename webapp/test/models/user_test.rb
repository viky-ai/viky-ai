require 'test_helper'

class UserTest < ActiveSupport::TestCase

  test "non admin user" do
    assert !User.find_by_email('notconfirmed@viky.ai').admin?
    assert User.find_by_email('admin@viky.ai').admin?

    u = User.new(email: 'not-admin@viky.ai', password: 'Hello baby', username: 'mrwho')
    assert u.save
    assert !u.admin?
  end


  test "username uniqueness" do
    admin = User.find_by_email('admin@viky.ai')
    admin.username = "admin"
    assert admin.save

    confirmed = User.find_by_email('confirmed@viky.ai')
    confirmed.username = "admin"
    assert !confirmed.save
    assert_equal ["has already been taken"], confirmed.errors.messages[:username]
  end


  test "username cleanup" do
    admin = User.find_by_email('admin@viky.ai')
    admin.username = "Admin Yeah ' Toto"
    assert admin.save
    assert_equal "admin-yeah-toto", admin.username

    admin.username = "  "
    assert !admin.save
    assert_equal "", admin.username

    admin.username = "\" 1   2 3 ABC !?/^äù"
    assert admin.save
    assert_equal "1-2-3-abc-au", admin.username
  end


  test "search & order_by" do
    s = Backend::UserSearch.new
    expected = [
      'locked@viky.ai',
      'admin@viky.ai',
      'confirmed@viky.ai',
      'invited@viky.ai',
      'notconfirmed@viky.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)


    s = Backend::UserSearch.new(sort_by: 'email')
    expected = [
      'admin@viky.ai',
      'confirmed@viky.ai',
      'invited@viky.ai',
      'locked@viky.ai',
      'notconfirmed@viky.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)
  end


  test "search & status" do
    s = Backend::UserSearch.new
    assert_equal 5, User.search(s.options).count

    s = Backend::UserSearch.new(status: 'confirmed')
    assert_equal 3, User.search(s.options).count
    expected = [
      'locked@viky.ai',
      'admin@viky.ai',
      'confirmed@viky.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)

    s = Backend::UserSearch.new(status: 'not-confirmed')
    assert_equal 2, User.search(s.options).count
    expected = [
      'invited@viky.ai',
      'notconfirmed@viky.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)

    s = Backend::UserSearch.new(status: 'locked')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@viky.ai", User.search(s.options).first.email
  end


  test "search by email" do
    s = Backend::UserSearch.new
    assert_equal 5, User.search(s.options).count

    s = Backend::UserSearch.new(email: 'lock')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@viky.ai", User.search(s.options).first.email
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
    User.invite!({email: "tester_nil_1@viky.ai"}, users(:admin))
    User.invite!({email: "tester_nil_2@viky.ai"}, users(:admin))

    assert !User.find_by_email("tester_nil_1@viky.ai").nil?
    assert !User.find_by_email("tester_nil_2@viky.ai").nil?
  end


  test "Test user slug" do
    user = User.friendly.find("admin")
    assert_equal "admin@viky.ai", user.email

    user.username = 'new-admin'
    assert user.save

    user = User.friendly.find("admin")
    assert_equal "admin@viky.ai", user.email
    user = User.friendly.find("new-admin")
    assert_equal "admin@viky.ai", user.email

    user.username = 'new-new-admin'
    assert user.save
    user = User.friendly.find("admin")
    assert_equal "admin@viky.ai", user.email
    user = User.friendly.find("new-admin")
    assert_equal "admin@viky.ai", user.email
    user = User.friendly.find("new-new-admin")
    assert_equal "admin@viky.ai", user.email
  end


end
