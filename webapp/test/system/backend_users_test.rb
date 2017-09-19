require "application_system_test_case"

class BackendUsersTest < ApplicationSystemTestCase

  test 'User index not allowed if user is not logged in' do
    visit backend_users_path

    assert page.has_content?('Please, log in before continuing.')
    assert_equal '/users/sign_in', current_path
  end


  test 'User index not allowed if user is not admin' do
    visit new_user_session_path

    fill_in 'Email', with: 'confirmed@voqal.ai'
    fill_in 'Password', with: 'BimBamBoom'

    click_button 'Log in'

    visit backend_users_path
    assert page.has_content?('You do not have permission to access this interface.')
    assert_equal '/', current_path
  end


  test 'Successful log in' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path
    assert page.has_content?('BACKEND USER INDEX')
    assert_equal '/backend/users', current_path
  end

end
