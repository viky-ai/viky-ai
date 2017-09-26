require "application_system_test_case"

class ProfileTest < ApplicationSystemTestCase

  test "Profile access" do
    admin_login
    click_link "admin@voqal.ai"
    assert page.has_content?("Authentication parameters")
  end


  test "Update Authentication parameters without any change" do
    admin_login
    click_link "admin@voqal.ai"
    fill_in 'Password', with: ''
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert !page.has_content?("Password is too short (minimum is 6 characters)")
  end


  test "Update Authentication parameters - Change password" do
    admin_login
    click_link "admin@voqal.ai"
    fill_in 'Password', with: 'short'
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert page.has_content?("Password is too short (minimum is 6 characters)")

    fill_in 'Password', with: 'shortshort'
    click_button 'Update profile'
    assert page.has_content?("Authentication parameters")
    assert !page.has_content?("Password is too short (minimum is 6 characters)")

    click_link "Logout"
    click_link "Log in"
    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'shortshort'
    click_button 'Log in'

    assert page.has_content?("Signed in successfully.")
  end


  test "Update Authentication parameters - Change email" do
    admin_login
    click_link "admin@voqal.ai"
    fill_in 'Email', with: 'admin_new@voqal.ai'
    click_button 'Update profile'

    assert page.has_content?("Currently waiting confirmation for: admin_new@voqal.ai")
  end


  test "Delete my account" do
    admin_login
    click_link 'admin@voqal.ai'
    click_link 'I want delete my account'

    assert page.has_content?("Are you sure?")

    fill_in 'validation', with: 'DEL'
    click_button('Delete my account')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')


    fill_in 'validation', with: 'DELETE'
    click_button('Delete my account')

    assert page.has_content?('Your account has been succefully delete. ByeBye.')
  end

end
