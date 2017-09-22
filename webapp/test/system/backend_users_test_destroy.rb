require "application_system_test_case"

class BackendUsersTestDestroy < ApplicationSystemTestCase

  test 'Destroy with confirmation' do
    admin_login

    click_link('Backend')
    assert page.has_content?('4 users')

    all('a.btn--destructive').last.click

    assert page.has_content?('Are you sure?')
    click_button('Delete')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', with: 'DELETE'
    click_button('Delete')
    assert page.has_content?('User with the email: notconfirmed@voqal.ai has successfully been deleted.')
  end

end
