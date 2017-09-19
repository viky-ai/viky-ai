require 'test_helper'

class UserTest < ActiveSupport::TestCase

  test "non admin user" do
    assert !User.find_by_email('notconfirmed@voqal.ai').admin?
    assert User.find_by_email('admin@voqal.ai').admin?

    u = User.new(email: 'not-admin@voqal.ai', password: 'Hello baby')
    assert u.save
    assert !u.admin?
  end


  test "search & order_by" do
    s = Backend::UserSearch.new
    expected = [
      'locked@voqal.ai',
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)


    s = Backend::UserSearch.new(sort_by: 'email')
    expected = [
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'locked@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)
  end


  test "search & status" do
    s = Backend::UserSearch.new
    assert_equal 4, User.search(s.options).count

    s = Backend::UserSearch.new(status: 'confirmed')
    assert_equal 3, User.search(s.options).count
    expected = [
      'locked@voqal.ai',
      'admin@voqal.ai',
      'confirmed@voqal.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)

    s = Backend::UserSearch.new(status: 'not-confirmed')
    assert_equal 1, User.search(s.options).count
    assert_equal "notconfirmed@voqal.ai", User.search(s.options).first.email

    s = Backend::UserSearch.new(status: 'locked')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@voqal.ai", User.search(s.options).first.email
  end


  test "search by email" do
    s = Backend::UserSearch.new
    assert_equal 4, User.search(s.options).count

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

end
