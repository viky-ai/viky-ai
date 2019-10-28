require "application_system_test_case"

class BackendUsersTest < ApplicationSystemTestCase


  test "User index not allowed if user is not logged in" do
    visit backend_users_path

    assert has_text?("Please, log in before continuing.")
    assert_equal "/users/sign_in", current_path
  end


  test "User index not allowed if user is not admin" do
    login_as "confirmed@viky.ai", "BimBamBoom"
    visit backend_users_path

    assert has_text?("You do not have permission to access this interface.")
    assert_equal "/agents", current_path
  end


  test "Successful login" do
    admin_login
    visit backend_users_path

    assert has_css?(".backend-users .user", count: 7)

    assert_equal "/backend/users", current_path
  end


  test "Users can be filtered" do
    admin_login
    visit backend_users_path

    find(".dropdown__trigger", text: "All").click
    all(".dropdown__content li").each do |filter_name|
      next if filter_name.text
      find(".dropdown__content", text: filter_name.text).click
      assert has_css?(".backend-users .user", count: 1)
      assert_equal "/backend/users", current_path
      find(".dropdown__trigger", text: filter_name.text).click
    end
  end


  test "Users can be sorted by email" do
    admin_login
    visit backend_users_path

    find(".dropdown__trigger", text: "Sort by last login").click
    find(".dropdown__content", text: "Sort by email").click
    expected = [
      "admin@viky.ai",
      "confirmed@viky.ai",
      "edit_on_agent_weather@viky.ai",
      "invited@viky.ai",
      "locked@viky.ai",
      "notconfirmed@viky.ai",
      "show_on_agent_weather@viky.ai"
    ]

    find(".field .control:last-child .dropdown__trigger a").assert_text "Sort by email"

    assert_equal expected, all(".user__info small").collect(&:text)
  end


  test "Users can be found by search" do
    admin_login
    visit backend_users_path

    assert has_text?("Backend / User management")

    fill_in "search_query", with: "ocked"
    find_button(id: "search").click

    assert has_css?(".backend-users .user", count: 1)
    assert has_text?("locked@viky.ai")

    assert_equal "/backend/users", current_path
  end


  test "Users can be found by search trimmed" do
    admin_login
    visit backend_users_path

    assert has_text?("Backend / User management")

    fill_in "search_query", with: " ocked   "
    find_button(id: "search").click

    assert has_css?(".backend-users .user", count: 1)
    assert has_text?("locked@viky.ai")

    assert_equal "/backend/users", current_path
  end


  test "Destroy user without username" do
    before_count = User.count

    admin_login
    visit backend_users_path

    assert has_css?(".backend-users .user", count: before_count)

    all("a.btn--destructive").last.click

    assert has_text?("Are you sure?")
    click_button("Delete")
    assert has_text?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "dElEtE"
    click_button("Delete")
    assert has_text?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "DELETE"
    click_button("Delete")
    assert has_text?("User with the email: notconfirmed@viky.ai has successfully been deleted.")
    assert_equal "/backend/users", current_path
    assert has_css?(".backend-users .user", count: before_count - 1)
  end


  test "Destroy user with username" do
    before_count = User.count
    admin_login
    visit backend_users_path

    assert has_css?(".backend-users .user", count: before_count)

    all("a.btn--destructive")[2].click
    assert has_text?("Are you sure?")
    assert has_text?("You're about to delete user with the email: invited@viky.ai.")
    fill_in "validation", with: "DELETE"
    click_button("Delete")
    assert has_text?("User with the email: invited@viky.ai has successfully been deleted.")
    assert_equal "/backend/users", current_path
    assert has_css?(".backend-users .user", count: before_count - 1)
  end


  test "An invitation can be sent by administrators only" do
    visit new_user_invitation_path

    assert has_text? "You need to sign in or sign up before continuing."

    login_as "confirmed@viky.ai", "BimBamBoom"

    assert_equal "/agents", current_path
    assert has_text?("Signed in successfully.")

    visit new_user_invitation_path
    assert has_text? "You do not have permission to access this interface."

    logout
    login_as "admin@viky.ai", "AdminBoom"

    assert_equal "/agents", current_path
    assert has_text?("Signed in successfully.")

    visit new_user_invitation_path
    fill_in "Email", with: "bibibubu@bibibubu.org"
    click_button "Send invitation"
    assert has_text?("An invitation email has been sent to bibibubu@bibibubu.org.")
  end


  test "Invitations can be resent to not confirmed users only" do
    admin_login
    visit backend_users_path

    assert has_css?(".backend-users .user", count: 7)

    all(".backend-users > div").collect(&:text).each do |content|
      if content.include?("Not confirmed")
        assert content.include?("Re-invite")
      else
        assert_not content.include?("Re-invite")
      end
    end

    first(".backend-users .btn--primary", text: "Re-invite").click
    assert has_text?("An invitation email has been sent to")
    assert_equal "/backend/users", current_path
  end


  test "User switch" do
    admin_login
    within("nav") do
      assert has_text?("admin")
    end
    visit backend_users_path

    assert has_text?("edit_on_agent_weather@viky.ai")
    within(".backend-users") do
      first(".btn--primary").click
    end
    assert has_text?("Agents")
    within("nav") do
      assert has_text?("edit_on_agent_weather")
      assert has_text?("Switched")
    end

    visit "backend/users"
    assert has_text?("You do not have permission to access this interface.")

    click_link("Comeback")
    assert has_text?("Agents")
    within("nav") do
      assert has_text?("admin")
    end
  end


  test "Quota toggle" do
    Feature.with_quota_enabled do
      admin_login
      visit backend_users_path

      assert has_css?('.btn--toggle.btn--toggle-on', count: 7)
      within('.backend-users') do
        first('.btn--toggle').click
      end
      sleep(0.5)
      assert has_css?('.btn--toggle.btn--toggle-on', count: 6)
    end
  end


  test "Chatbot toggle" do
    Feature.with_chatbot_enabled do
      admin_login
      visit backend_users_path

      assert has_css?('.btn--toggle.btn--toggle-on', count: 0)
      within('.backend-users') do
        first('.btn--toggle').click
      end
      sleep(0.5)
      assert has_css?('.btn--toggle.btn--toggle-on', count: 1)
    end
  end

end
