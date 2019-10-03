require "application_system_test_case"

class BackendUsersTest < ApplicationSystemTestCase


  test "User index not allowed if user is not logged in" do
    visit backend_users_path

    assert has_content?("Please, log in before continuing.")
    assert_equal "/users/sign_in", current_path
  end


  test "User index not allowed if user is not admin" do
    login_as "confirmed@viky.ai", "BimBamBoom"

    visit backend_users_path
    assert has_content?("You do not have permission to access this interface.")
    assert_equal "/agents", current_path
  end


  test "Successful login" do
    admin_login

    visit backend_users_path
    assert has_content?("7 users")
    assert_equal "/backend/users", current_path
  end


  test "Users can be filtered" do
    admin_login

    click_link("Users management")

    find(".dropdown__trigger", text: "All").click
    all(".dropdown__content li").each do |filter_name|
      next if filter_name.text
      find(".dropdown__content", text: filter_name.text).click
      assert has_content?("1 user")
      assert_equal "/backend/users", current_path
      find(".dropdown__trigger", text: filter_name.text).click
    end
  end


  test "Users can be sorted by email" do
    admin_login

    click_link("Users management")

    find(".dropdown__trigger", text: "Sort by last login").click
    find(".dropdown__content", text: "Sort by email").click
    expected = [
      "admin",
      "confirmed",
      "edit_on_agent_weather",
      "invited@viky.ai",
      "locked",
      "notconfirmed@viky.ai",
      "show_on_agent_weather"
    ]

    find(".field .control:last-child .dropdown__trigger a").assert_text "Sort by email"

    assert_equal expected, (all("tbody tr").map {|tr|
      tr.all("td").first.text.split(" ").first
    })
  end


  test "Users can be found by search" do
    admin_login

    click_link("Users management")

    assert has_content?("Backend / User management")

    fill_in "search_query", with: "ocked"
    find_button(id: "search").click

    assert has_content?("1 user")
    assert has_content?("locked@viky.ai")

    assert_equal "/backend/users", current_path
  end


  test "Users can be found by search trimmed" do
    admin_login

    click_link("Users management")

    assert has_content?("Backend / User management")

    fill_in "search_query", with: " ocked   "
    find_button(id: "search").click

    assert has_content?("1 user")
    assert has_content?("locked@viky.ai")

    assert_equal "/backend/users", current_path
  end


  test "Destroy user without username" do
    before_count = User.count

    admin_login

    click_link("Users management")
    assert has_content?("#{before_count} users")

    all("a.btn--destructive").last.click

    assert has_content?("Are you sure?")
    click_button("Delete")
    assert has_content?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "dElEtE"
    click_button("Delete")
    assert has_content?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "DELETE"
    click_button("Delete")
    assert has_content?("User with the email: notconfirmed@viky.ai has successfully been deleted.")
    assert_equal "/backend/users", current_path
    assert_equal before_count - 1, User.count
  end


  test "Destroy user with username" do
    before_count = User.count
    admin_login

    click_link("Users management")
    assert has_content?("#{before_count} users")

    all("a.btn--destructive")[2].click
    assert has_content?("Are you sure?")
    assert has_content?("You're about to delete user with the email: invited@viky.ai.")
    fill_in "validation", with: "DELETE"
    click_button("Delete")
    assert has_content?("User with the email: invited@viky.ai has successfully been deleted.")
    assert_equal "/backend/users", current_path
    assert_equal before_count - 1, User.count
  end


  test "An invitation can be sent by administrators only" do
    visit new_user_invitation_path
    assert has_content? "You need to sign in or sign up before continuing."

    login_as "confirmed@viky.ai", "BimBamBoom"

    assert_equal "/agents", current_path
    assert has_content?("Signed in successfully.")

    visit new_user_invitation_path
    assert has_content? "You do not have permission to access this interface."

    logout
    login_as "admin@viky.ai", "AdminBoom"

    assert_equal "/agents", current_path
    assert has_content?("Signed in successfully.")

    visit new_user_invitation_path
    fill_in "Email", with: "bibibubu@bibibubu.org"
    click_button "Send invitation"
    assert has_content?("An invitation email has been sent to bibibubu@bibibubu.org.")
  end


  test "Invitations can be resent to not confirmed users only" do
    admin_login

    click_link("Users management")

    assert has_content?("7 users")

    all("tbody tr").each do |tr|
      user_line = tr.all("td").map {|td| td.text}.join
      if user_line.include?("Not confirmed")
        assert user_line.include?("Re-invite")
      else
        assert user_line.include?("Confirmed")
        assert_not user_line.include?("Re-invite")
      end
    end

    first("table .btn--primary", text: "Re-invite").click
    assert has_content?("An invitation email has been sent to")
    assert_equal "/backend/users", current_path
  end


  test "User switch" do
    admin_login
    within("nav") do
      assert has_text?("admin")
    end

    click_link("Users management")
    assert has_text?("edit_on_agent_weather@viky.ai")
    within("table") do
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

      click_link("Users management")
      assert has_content?('7 users')

      assert_equal 7, all('.btn--toggle.btn--toggle-on').size
      within('table') do
        first('.btn--toggle').click
      end
      sleep(0.5)
      assert_equal 6, all('.btn--toggle.btn--toggle-on').size
    end
  end

end
