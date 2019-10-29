require 'test_helper'

class UserTest < ActiveSupport::TestCase

  test "non admin user" do
    assert_not User.find_by_email('notconfirmed@viky.ai').admin?
    assert User.find_by_email('admin@viky.ai').admin?

    u = User.new(email: 'not-admin@viky.ai', password: 'Hello baby', username: 'mrwho')
    assert u.save
    assert_not u.admin?
  end


  test "username uniqueness" do
    admin = User.find_by_email('admin@viky.ai')
    admin.username = "admin"
    assert admin.save

    confirmed = User.find_by_email('confirmed@viky.ai')
    confirmed.username = "admin"
    assert_not confirmed.save
    assert_equal ["has already been taken"], confirmed.errors.messages[:username]
  end


  test "username cleanup" do
    admin = User.find_by_email('admin@viky.ai')
    admin.username = "Admin Yeah ' Toto"
    assert admin.save
    assert_equal "admin-yeah-toto", admin.username

    admin.username = "  "
    assert_not admin.save

    admin.username = "\" 1   2 3 ABC !?/^äù"
    assert admin.save
    assert_equal "1-2-3-abc-au", admin.username
  end


  test "search & order_by" do
    s = Backend::UserSearch.new
    expected = [
      'edit_on_agent_weather@viky.ai',
      'show_on_agent_weather@viky.ai',
      'locked@viky.ai',
      'admin@viky.ai',
      'confirmed@viky.ai',
      'invited@viky.ai',
      'notconfirmed@viky.ai',
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)


    s = Backend::UserSearch.new(sort_by: 'email')
    expected = [
      'admin@viky.ai',
      'confirmed@viky.ai',
      'edit_on_agent_weather@viky.ai',
      'invited@viky.ai',
      'locked@viky.ai',
      'notconfirmed@viky.ai',
      'show_on_agent_weather@viky.ai'
    ]
    assert_equal expected, User.search(s.options).all.collect(&:email)
  end


  test "search & status" do
    s = Backend::UserSearch.new
    assert_equal 7, User.search(s.options).count

    s = Backend::UserSearch.new(status: 'confirmed')
    assert_equal 5, User.search(s.options).count
    expected = [
      'edit_on_agent_weather@viky.ai',
      'show_on_agent_weather@viky.ai',
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
    assert_equal 7, User.search(s.options).count

    s = Backend::UserSearch.new(query: 'lock')
    assert_equal 1, User.search(s.options).count
    assert_equal "locked@viky.ai", User.search(s.options).first.email
  end


  test "search empty" do
    s = Backend::UserSearch.new
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email')
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email', query: "  ")
    assert s.empty?

    s = Backend::UserSearch.new(sort_by: 'email', query: "locked")
    assert_not s.empty?
  end


  test "invitation status" do
    u = users(:invited)
    travel_to(u.invitation_sent_at + User.invite_for + 1.seconds)
    assert_equal :expired, u.invitation_status
    travel_to(u.invitation_sent_at + User.invite_for - 1.seconds)
    assert_equal :valid, u.invitation_status
    travel_back
  end


  test "Users with blank username are allowed during invitation" do
    User.invite!({email: "tester_nil_1@viky.ai"}, users(:admin))
    User.invite!({email: "tester_nil_2@viky.ai"}, users(:admin))

    assert_not_nil User.find_by_email("tester_nil_1@viky.ai")
    assert_not_nil User.find_by_email("tester_nil_2@viky.ai")
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


  test "Test user agent rights" do
    admin_user = users(:admin)
    weather_agent = agents(:weather)
    weather_confirmed_agent = agents(:weather_confirmed)

    assert admin_user.can?(:edit, weather_agent)
    assert admin_user.can?(:show, weather_agent)
    assert admin_user.owner?(weather_agent)

    assert_not admin_user.can?(:delete, weather_agent)

    assert_not admin_user.can?(:edit, weather_confirmed_agent)
    assert_not admin_user.can?(:show, weather_confirmed_agent)
    assert_not admin_user.owner?(weather_confirmed_agent)

    show_on_agent_weather_user = users(:show_on_agent_weather)
    assert_not show_on_agent_weather_user.can?(:edit, weather_agent)
    assert show_on_agent_weather_user.can?(:show, weather_agent)
    assert_not show_on_agent_weather_user.owner?(weather_agent)

    assert_not show_on_agent_weather_user.can?(:edit, weather_confirmed_agent)
    assert_not show_on_agent_weather_user.can?(:show, weather_confirmed_agent)
    assert_not show_on_agent_weather_user.owner?(weather_confirmed_agent)

    edit_on_agent_weather_user = users(:edit_on_agent_weather)
    assert edit_on_agent_weather_user.can?(:edit, weather_agent)
    assert edit_on_agent_weather_user.can?(:show, weather_agent)
    assert_not edit_on_agent_weather_user.owner?(weather_agent)

    assert_not edit_on_agent_weather_user.can?(:edit, weather_confirmed_agent)
    assert_not edit_on_agent_weather_user.can?(:show, weather_confirmed_agent)
    assert_not edit_on_agent_weather_user.owner?(weather_confirmed_agent)
  end


  test "Can not destroy if user own agent" do
    admin_user = users(:admin)

    assert_equal 6, Membership.all.count
    assert_not admin_user.destroy

    expected = ["You must delete all the agents you own"]
    assert_equal expected, admin_user.errors.full_messages

    assert_not admin_user.can_be_destroyed?
  end


  test "Can destroy if user is only a collaborator" do
    collaborator = users(:show_on_agent_weather)

    assert_equal 4, Agent.all.count
    assert_equal 6, Membership.all.count

    assert collaborator.destroy

    assert_equal 4, Agent.all.count
    assert_equal 5, Membership.all.count
  end


  test 'Test user slug generation' do
    admin = users(:admin)
    assert_equal 'admin', admin.slug
  end


  test 'Users expression count' do
    admin = users(:admin)
    assert_equal 10, admin.expressions_count

    user = users(:confirmed)
    assert_equal 0, user.expressions_count
  end


  test 'Quota exceeded' do
    Feature.with_quota_enabled do
      Quota.stubs(:expressions_limit).returns(10)
      assert users(:admin).quota_exceeded?
      assert_not users(:confirmed).quota_exceeded?
    end
  end


  test 'Agents by expressions count' do
    admin = users(:admin)
    agent_expressions = admin.agents_by_expressions_count
    assert_equal 2, agent_expressions.count
    assert_equal agents(:weather).id, agent_expressions.first.id
    assert_equal 7, agent_expressions.first.total

    assert_equal agents(:terminator).id, agent_expressions.last.id
    assert_equal 3, agent_expressions.last.total

    user = users(:confirmed)
    agent_expressions = user.agents_by_expressions_count
    assert_equal 1, agent_expressions.count
    assert_equal agents(:weather_confirmed).id, agent_expressions.first.id
    assert_equal 0, agent_expressions.first.total
  end


  test 'Expressions count per owners' do
    result = User.expressions_count_per_owners
    assert_equal 3, result.count

    assert_equal result.first.id, users(:admin).id
    assert_equal 10, result.first.total

    assert_equal result.second.id, users(:locked).id
    assert_equal 4, result.second.total

    assert_equal result.last.id, users(:confirmed).id
    assert_equal 0, result.last.total
  end


  test 'entities, interpretations, expressions count' do
    assert_equal 4, users(:admin).entities_count
    assert_equal 6, users(:admin).interpretations_count
    assert_equal 10, users(:admin).expressions_count
  end


  test 'Quota ignored for user' do
    Feature.with_quota_enabled do
      Quota.stubs(:expressions_limit).returns(10)

      admin = users(:admin)
      admin.quota_enabled = false

      assert admin.save
      assert_equal 10, admin.expressions_count
      assert_not users(:admin).quota_exceeded?

      admin.quota_enabled = true
      assert admin.save
      assert users(:admin).quota_exceeded?
    end
  end


  test 'Trigger an NLP sync of every agent when the username change' do
    Nlp::Package.any_instance.expects(:push).twice

    user = users(:admin)
    assert_equal 2, user.agents.count
    user.username = 'administrator'
    assert user.save

    user.bio = 'My life is amazing !'
    assert user.save
  end
end
