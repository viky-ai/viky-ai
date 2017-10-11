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
    admin_login

    visit backend_users_path
    assert page.has_content?('5 users')
    assert_equal '/backend/users', current_path
  end


  test 'Users can be filtered' do
    admin_login

    click_link('Backend')

    find('.dropdown__trigger', text: 'All').click
    all('.dropdown__content li').each do |filter_name|
      next if filter_name.text
      find('.dropdown__content', text: filter_name.text).click
      assert page.has_content?('1 user')
      assert_equal '/backend/users', current_path
      find('.dropdown__trigger', text: filter_name.text).click
    end
  end


  test 'Users can be sorted by email' do
    admin_login

    click_link('Backend')

    find('.dropdown__trigger', text: 'Sort by last log in').click
    find('.dropdown__content', text: 'Sort by email').click
    expected = [
      'admin@voqal.ai',
      'confirmed@voqal.ai',
      'invited@voqal.ai',
      'locked@voqal.ai',
      'notconfirmed@voqal.ai'
    ]

    find(".field .control:last-child .dropdown__trigger a").assert_text "Sort by email"

    assert_equal expected, all("tbody tr").map {|tr|
      tr.all('td').first.text.split(' ').first
    }
  end


  test 'Users can be found by email' do
    admin_login

    click_link('Backend')

    fill_in 'search_email', with: 'ocked'
    click_button '#search'

    assert page.has_content?('1 user')
    assert page.has_content?('locked@voqal.ai')

    assert_equal '/backend/users', current_path
  end


  test 'Destroy user without username' do
    before_count = User.count

    admin_login

    click_link('Backend')
    assert page.has_content?('5 users')

    all('a.btn--destructive').last.click

    assert page.has_content?('Are you sure?')
    click_button('Delete')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', with: 'dElEtE'
    click_button('Delete')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', with: 'DELETE'
    click_button('Delete')
    assert page.has_content?('User with the email: notconfirmed@voqal.ai has successfully been deleted.')
    assert_equal '/backend/users', current_path
    assert_equal before_count - 1, User.count
  end


  test 'Destroy user with username' do
    before_count = User.count
    admin_login

    click_link('Backend')
    assert page.has_content?('5 users')

    all('a.btn--destructive')[1].click
    assert page.has_content?('Are you sure?')
    assert page.has_content?("You're about to delete user with the email: locked@voqal.ai.")
    fill_in 'validation', with: 'DELETE'
    click_button('Delete')
    assert page.has_content?('User with the email: locked@voqal.ai has successfully been deleted.')
    assert_equal '/backend/users', current_path
    assert_equal before_count - 1, User.count
  end


  test "An invitation can be sent by administrators only" do
    visit new_user_invitation_path
    assert page.has_content? "You need to sign in or sign up before continuing."

    visit new_user_session_path
    fill_in 'Email', with: 'confirmed@voqal.ai'
    fill_in 'Password', with: 'BimBamBoom'
    click_button 'Log in'

    assert_equal '/', current_path
    assert page.has_content? "You do not have permission to access this interface."

    first('.nav__footer svg').click # Logout

    visit new_user_session_path
    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'
    click_button 'Log in'

    assert_equal '/', current_path
    assert page.has_content?("Signed in successfully.")

    visit new_user_invitation_path
    fill_in 'Email', with: 'bibibubu@bibibubu.org'
    click_button 'Send invitation'
    assert page.has_content?("An invitation email has been sent to bibibubu@bibibubu.org.")
  end


  test 'Invitations can be resent to not confirmed users only' do
    admin_login

    click_link('Backend')

    assert page.has_content?('5 users')

    all("tbody tr").each do |tr|
      user_line = tr.all('td').map {|td| td.text}.join
      if user_line.include?('Not confirmed')
        assert user_line.include?('Re-invite')
      else
        assert user_line.include?('Confirmed')
        assert !user_line.include?('Re-invite')
      end
    end

    first('table .btn--primary', text: 'Re-invite').click
    assert page.has_content?('An invitation email has been sent to')
    assert_equal '/backend/users', current_path
  end

end
