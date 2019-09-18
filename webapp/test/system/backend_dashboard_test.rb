require "application_system_test_case"

class BackendDashboardTest < ApplicationSystemTestCase

  test 'User index not allowed if user is not logged in' do
    visit backend_dashboard_path
    assert page.has_content?('Please, log in before continuing.')
    assert_equal '/users/sign_in', current_path
  end


  test 'User index not allowed if user is not admin' do
    login_as 'confirmed@viky.ai', 'BimBamBoom'
    visit backend_dashboard_path
    assert page.has_content?('You do not have permission to access this interface.')
    assert_equal '/agents', current_path
  end


  test 'Successful access' do
    admin_login

    visit backend_dashboard_path
    assert_equal '/backend/dashboard', current_path
    assert page.has_content?('3 Agents')
    assert page.has_content?('One public agent')
    assert page.has_content?('6 Interpretations')
    assert page.has_content?('5 intents')
    assert page.has_content?('4 Entities')
    assert page.has_content?('4 entities lists')
    assert page.has_content?('0 Test')
    assert page.has_content?('1 Readme')
    assert page.has_content?('33% of agents')
    assert page.has_content?('2 Bots')
    assert page.has_content?('One WIP')
    assert page.has_content?('7 Users')
    assert page.has_content?('One administrator')

    assert page.has_content?('Agents with most interpretations')
    assert page.has_content?('Agents with most entities')
    assert page.has_content?('Agents most used as dependency')
    assert page.has_content?('Agents with most tests in failure')
    assert page.has_content?('Users with most requests in the last 30 days')
    assert page.has_content?('Users with most formulations and entities')
  end

end
