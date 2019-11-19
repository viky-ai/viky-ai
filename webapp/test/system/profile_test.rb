require "application_system_test_case"

class ProfileTest < ApplicationSystemTestCase

  def go_to_profile
    admin_login
    within(".h-nav") do
      click_link "admin"
    end
    assert has_text?("Your profile")
  end


  test "Profile change name, username, bio" do
    go_to_profile
    click_link "Edit your profile"

    within ".test-profile-edit-global" do
      fill_in "Name", with: "Batman and Robin"
      fill_in "Username", with: "batman"
      fill_in "Bio", with: "blah blah blah"
      click_button "Update"
    end

    assert has_field?("Name", with: "Batman and Robin")
    assert has_field?("Username", with: "batman")
    assert has_field?("Bio", with: "blah blah blah")
  end


  test "Profile add avatar and remove" do
    go_to_profile
    click_link "Edit your profile"

    assert has_text?("Your avatar")

    within(".test-profile-edit-avatar") do
      assert find(".avatar img")["src"].include? "default"

      file = build_fixture_files_path('avatar_upload.png')
      attach_file("upload_image", file, make_visible: true)
      click_button "Update"

      assert find(".avatar img")["src"].include? "square"
      check("Remove")
      click_button "Update"

      assert find(".avatar img")["src"].include? "default"
    end
  end


  test "Profile avatar with not permitted format" do
    go_to_profile
    click_link "Edit your profile"

    assert has_text?("Your avatar")

    within(".test-profile-edit-avatar") do
      assert find(".avatar img")["src"].include? "default"

      file = build_fixture_files_path('avatar_upload.txt')
      attach_file("upload_image", file, make_visible: true)
      click_button "Update"

      assert find(".avatar img")["src"].include? "default"

      assert has_text?("Avatar type must be one of: image/jpeg, image/png, image/gif")
    end
  end


  test "Update Authentication parameters without any change" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Password", with: ""
      click_button "Update"
      assert has_no_text?("Password is too short (minimum is 6 characters)")
    end

  end


  test "Update Authentication parameters - Change password" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Password", with: "short"
      click_button "Update"

      assert has_text?("Password is too short (minimum is 6 characters)")

      fill_in "Password", with: "shortshort"
      click_button "Update"

      assert has_no_text?("Password is too short (minimum is 6 characters)")
    end

    logout

    fill_in "Email", with: "admin@viky.ai"
    fill_in "Password", with: "shortshort"
    click_button "Log in"

    assert has_text?("Signed in successfully.")
  end


  test "Update Authentication parameters - Change email" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Email", with: "admin_new@viky.ai"
      click_button "Update"
      assert has_text?("Currently waiting confirmation for: admin_new@viky.ai")
    end
  end


  test "Delete my account not available" do
    go_to_profile
    click_link "Edit your profile"
    within(".test-profile-edit-destroy") do
      assert has_text?("You must transfert your ownerships or delete all agents you own in order to delete your account.")
    end
  end


  test "Delete my account" do
    # Make user deletable
    agent = agents(:weather)
    agent.memberships.where.not(rights: "all").each do |m|
      assert m.destroy
    end
    assert agent.destroy

    agent = agents(:terminator)
    agent.memberships.where.not(rights: "all").each do |m|
      assert m.destroy
    end
    assert agent.destroy

    agent = agents(:cities)
    agent.memberships.where.not(rights: "all").each do |m|
      assert m.destroy
    end
    assert agent.destroy

    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-destroy") do
      click_link "I want to delete my account"
    end

    assert has_text?("Are you sure?")

    fill_in "validation", with: "DEL"
    click_button("Delete my account")
    assert has_text?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "DELETE"
    click_button("Delete my account")

    assert has_text?("Your account has been successfully deleted. Bye bye.")
  end

end
