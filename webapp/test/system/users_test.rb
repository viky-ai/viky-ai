require "application_system_test_case"

class UsersTest < ApplicationSystemTestCase

  test 'creating a new User' do
    # Visit the User Registration Page
    visit new_user_registration_path

    # Click the Sign Up Button with an empty form
    find('input[value="Sign up"]').click
    assert_selector 'h2', text: '2 errors prohibited this user from being saved:'

    # Click the Sign Up Button with form filled in
    fill_in 'Email', with: 'superman@voqal.ai'
    fill_in 'Password', with: 'supersecret'
    fill_in 'Password confirmation', with: 'supersecret'
    find('input[value="Sign up"]').click
    assert_selector '.notice', text: 'A message with a confirmation link has been sent to your email address. Please follow the link to activate your account.'
  end

end
