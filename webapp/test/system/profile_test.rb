require "application_system_test_case"

class ProfileTest < ApplicationSystemTestCase

  def go_to_profile
    admin_login
    within(".nav") do
      click_link "admin"
    end
    assert page.has_text?("Your profile")
  end


  test "Display user's agents" do
    go_to_profile
    within(".agents-box-grid") do
      assert page.has_content?("My awesome weather bot")
      assert page.has_content?("T-800")
      assert page.has_no_content?("Weather bot")
    end
  end


  test "Edit profile access" do
    go_to_profile
    click_link "Edit your profile"

    assert page.has_content?("Authentication parameters")
  end


  test "Profile change Name, username, bio" do
    go_to_profile
    click_link "Edit your profile"

    fill_in 'Name', with: 'Batman and Robin'
    fill_in 'Username', with: 'batman'
    fill_in 'Bio', with: "blah blah blah"

    click_button 'Update profile'

    assert page.has_field?('Name', with: 'Batman and Robin')
    assert page.has_field?('Username', with: 'batman')
    assert page.has_field?('Bio', with: 'blah blah blah')
  end


  test "Profile add avatar and remove" do
    go_to_profile
    click_link "Edit your profile"

    within("main") do
      assert find(".avatar img")['src'].include? 'default'
    end

    file = File.join(Rails.root, 'test', 'fixtures', 'files', 'avatar_upload.png')

    # https://github.com/teampoltergeist/poltergeist/issues/866
    attach_file('profile_image', file).click

    click_button 'Update profile'

    within("main") do
      assert find(".avatar img")['src'].include? 'square'
    end

    assert page.has_field?('profile[remove_image]')

    check('Remove avatar')
    click_button 'Update profile'
    within("main") do
      assert find(".avatar img")['src'].include? 'default'
    end
  end


  test "Profile avatar with not permitted format" do
    go_to_profile
    click_link "Edit your profile"

    within("main") do
      assert find(".avatar img")['src'].include? 'default'
    end

    file = File.join(Rails.root, 'test', 'fixtures', 'files', 'avatar_upload.txt')

    # https://github.com/teampoltergeist/poltergeist/issues/866
    attach_file('profile_image', file).click

    click_button 'Update profile'

    within("main") do
      assert find(".avatar img")['src'].include? 'default'
    end

    assert page.has_content?("Image isn't of allowed type")
  end


  test "Update Authentication parameters without any change" do
    go_to_profile
    click_link "Edit your profile"

    fill_in 'Password', with: ''
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert !page.has_content?("Password is too short (minimum is 6 characters)")
  end


  test "Update Authentication parameters - Change password" do
    go_to_profile
    click_link "Edit your profile"

    fill_in 'Password', with: 'short'
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert page.has_content?("Password is too short (minimum is 6 characters)")

    fill_in 'Password', with: 'shortshort'
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert !page.has_content?("Password is too short (minimum is 6 characters)")

    first('.nav__footer svg').click # Logout
    click_link "Log in"
    fill_in 'Email', with: 'admin@viky.ai'
    fill_in 'Password', with: 'shortshort'
    click_button 'Log in'

    assert page.has_content?("Signed in successfully.")
  end


  test "Update Authentication parameters - Change email" do
    go_to_profile
    click_link "Edit your profile"

    fill_in 'Email', with: 'admin_new@viky.ai'
    click_button 'Update profile'

    assert page.has_content?("Currently waiting confirmation for: admin_new@viky.ai")
  end


  test "Delete my account not available" do
    go_to_profile
    click_link "Edit your profile"
    assert page.has_content?("You must delete all agents you own in order to delete your account.")
  end


  test "Delete my account" do
    # Make user deletable
    agent = agents(:weather)
    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end
    assert agent.destroy

    agent = agents(:terminator)
    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end
    assert agent.destroy

    go_to_profile
    click_link "Edit your profile"

    click_link 'I want to delete my account'

    assert page.has_content?("Are you sure?")

    fill_in 'validation', with: 'DEL'
    click_button('Delete my account')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')


    fill_in 'validation', with: 'DELETE'
    click_button('Delete my account')

    assert page.has_content?('Your account has been successfully deleted. Bye bye.')
  end

end
