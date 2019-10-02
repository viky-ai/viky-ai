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

    assert has_content?("Your avatar")

    within(".test-profile-edit-avatar") do
      assert find(".avatar img")["src"].include? "default"

      file = File.join(Rails.root, "test", "fixtures", "files", "avatar_upload.png")

      # Display import file imput in order to allow capybara attach_file
      execute_script("$('#upload_image').css('opacity','1')");

      attach_file("upload_image", file)

      click_button "Update"

      assert find(".avatar img")["src"].include? "square"
      assert has_field?("profile[remove_image]")
      check("Remove")
      click_button "Update"

      assert find(".avatar img")["src"].include? "default"
    end
  end


  test "Profile avatar with not permitted format" do
    go_to_profile
    click_link "Edit your profile"

    assert has_content?("Your avatar")

    within(".test-profile-edit-avatar") do
      assert find(".avatar img")["src"].include? "default"

      file = File.join(Rails.root, "test", "fixtures", "files", "avatar_upload.txt")

      # Display import file imput in order to allow capybara attach_file
      execute_script("$('#upload_image').css('opacity','1')");

      attach_file("upload_image", file)

      click_button "Update"

      assert find(".avatar img")["src"].include? "default"

      assert has_content?("Avatar type must be one of: image/jpeg, image/png, image/gif")
    end
  end


  test "Update Authentication parameters without any change" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Password", with: ""
      click_button "Update"
      assert has_no_content?("Password is too short (minimum is 6 characters)")
    end

  end


  test "Update Authentication parameters - Change password" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Password", with: "short"
      click_button "Update"

      assert has_content?("Password is too short (minimum is 6 characters)")

      fill_in "Password", with: "shortshort"
      click_button "Update"

      assert has_no_content?("Password is too short (minimum is 6 characters)")
    end

    logout

    fill_in "Email", with: "admin@viky.ai"
    fill_in "Password", with: "shortshort"
    click_button "Log in"

    assert has_content?("Signed in successfully.")
  end


  test "Update Authentication parameters - Change email" do
    go_to_profile
    click_link "Edit your profile"

    within(".test-profile-edit-auth") do
      fill_in "Email", with: "admin_new@viky.ai"
      click_button "Update"
      assert has_content?("Currently waiting confirmation for: admin_new@viky.ai")
    end
  end


  test "Delete my account not available" do
    go_to_profile
    click_link "Edit your profile"
    within(".test-profile-edit-destroy") do
      assert has_content?("You must delete all agents you own in order to delete your account.")
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

    assert has_content?("Are you sure?")

    fill_in "validation", with: "DEL"
    click_button("Delete my account")
    assert has_content?("Please enter the text exactly as it is displayed to confirm.")

    fill_in "validation", with: "DELETE"
    click_button("Delete my account")

    assert has_content?("Your account has been successfully deleted. Bye bye.")
  end

end
