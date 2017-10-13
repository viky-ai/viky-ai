require "application_system_test_case"

class AgentsShowTest < ApplicationSystemTestCase

  test 'No redirection when configuring from show' do
    go_to_agents_index
    click_link 'agent-admin-weather'
    assert page.has_text?('Agents / My awesome weather bot (admin/weather)')
    click_link 'Configure'
    assert page.has_text?('Configure agent')
    click_link 'Cancel'
    assert_equal '/agents/admin/weather', current_path

    click_link 'Configure'
    assert page.has_text?('Configure agent')
    fill_in 'Name', with: 'My new updated weather agent'
    click_button 'Update'
    assert page.has_text?('Agents / My new updated weather agent (admin/weather)')
    assert_equal '/agents/admin/weather', current_path
  end

end
