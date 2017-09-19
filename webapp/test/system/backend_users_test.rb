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
    assert page.has_content?('4 users')
    assert_equal '/backend/users', current_path
  end

  test 'Users can be filtered' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    ['not-confirmed', 'locked'].each do |filter_name|
      find("input[value='#{filter_name}']").click
      click_button 'Search'

      assert page.has_content?('1 user')
      assert_equal '/backend/users', current_path
    end
  end

  test 'Users can be sorted by last action and email' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    find('#search_sort_by').find(:xpath, 'option[1]').select_option
    click_button 'Search'

    expected = [
      'admin@voqal.ai',
      'locked@voqal.ai',
      'confirmed@voqal.ai',
      'notconfirmed@voqal.ai'
    ]

    assert_equal expected, all(".card-content h3").map {|h3| h3.text.split(' ').first}

    find('#search_sort_by').find(:xpath, 'option[2]').select_option
    click_button 'Search'

    expected = [
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'locked@voqal.ai',
      'notconfirmed@voqal.ai'
    ]

    assert_equal expected, all(".card-content h3").map {|h3| h3.text.split(' ').first}
  end

  test 'Users can be found by email' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    fill_in 'search_email', :with => 'ocked'
    click_button 'Search'

    assert page.has_content?('1 user')
    assert page.has_content?('locked@voqal.ai')

    assert_equal '/backend/users', current_path
  end

end
