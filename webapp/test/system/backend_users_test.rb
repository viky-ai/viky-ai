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
    assert page.has_content?('5 users')
    assert_equal '/backend/users', current_path
  end

  test 'Users can be filtered' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    find('.dropdown__trigger', text: 'All').click
    all('.dropdown__content li').each do |filter_name|
      next if filter_name.text
      find('.dropdown__content', text: filter_name.text).click
      assert page.has_content?('1 user')
      assert_equal '/backend/users', current_path
      find('.dropdown__trigger', text: filter_name.text).click
    end
  end

  test 'Users can be sorted by last action and email' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    find('.dropdown__trigger', text: 'Last log in').click
    find('.dropdown__content', text: 'Email').click
    expected = [
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'invited@voqal.ai',
      'locked@voqal.ai',
      'notconfirmed@voqal.ai'
    ]
    sleep 0.2
    assert_equal expected, all("tbody tr").map {|tr|
      tr.all('td').first.text.split(' ').first
    }
  end

  test 'Users can be found by email' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    fill_in 'search_email', :with => 'ocked'
    click_button '#search'

    assert page.has_content?('1 user')
    assert page.has_content?('locked@voqal.ai')

    assert_equal '/backend/users', current_path
  end

  test 'Users can be deleted' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    delete_buttons_before = all('.btn--destructive', text: 'Delete')
    before_count = delete_buttons_before.count
    delete_buttons_before[1].click
    fill_in 'validation', :with => 'dElEtE'
    click_button 'Delete'
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', :with => 'DELETE'
    click_button 'Delete'
    assert page.has_content?('has successfully been deleted.')

    assert_equal '/backend/users', current_path
    delete_buttons_after = all('.btn--destructive', text: 'Delete')
    assert_equal before_count - 1, delete_buttons_after.count
  end

  test 'Invitations can be resent to not confirmed users only' do
    visit new_user_session_path

    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'

    click_button 'Log in'

    visit backend_users_path

    all("tbody tr").each do |tr|
      user_line = tr.all('td').map {|td| td.text}.join
      if user_line.include?('Not confirmed')
        assert user_line.include?('Re-invite')
      else
        assert user_line.include?('Confirmed')
        assert !user_line.include?('Re-invite')
      end
    end

    all('.btn--primary', text: 'Re-invite').first.click
    sleep 0.5
    assert_equal '/backend/users', current_path
    assert page.has_content?('An invitation email has been sent to')
  end

end
