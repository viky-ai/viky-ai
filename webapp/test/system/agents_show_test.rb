require "application_system_test_case"

class AgentsShowTest < ApplicationSystemTestCase

  test 'Navigation to agent show' do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert page.has_text?('Agents / My awesome weather bot (admin/weather)')
    assert_equal '/agents/admin/weather', current_path
  end


  test 'No redirection when configuring from show' do
    go_to_agents_index
    click_link 'My awesome weather bot admin/weather'
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


  test 'Transfer agent ownership' do
    go_to_agents_index
    assert page.has_content?('T-800')
    click_link 'agent-admin-terminator'
    click_link 'Transfer ownership'
    within(".modal") do
      assert page.has_field? 'users_new_owner_id'
      assert page.has_button? 'Transfer'
    end
  end
end
