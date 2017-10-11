require "application_system_test_case"

class AgentsTest < ApplicationSystemTestCase

  test 'Navigation to agents index' do
    go_to_agents_index
    assert page.has_content?("Agents management")
    assert_equal "My awesome weather bot", first('.agent-box h2').text
  end

  test 'blank slate' do
    Agent.delete_all
    go_to_agents_index
    assert page.has_content?("Create your first agent")
  end

  test 'Button to delete agent is present' do
    go_to_agents_index
    assert page.has_link?("Delete")
  end

  test 'Delete with confirmation' do
    before_count = Agent.count
    go_to_agents_index
    click_link 'Delete'
    assert page.has_content?('Are you sure?')
    click_button('Delete')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')
    fill_in 'validation', with: 'dElEtE'
    click_button('Delete')
    assert page.has_content?('Please enter the text exactly as it is displayed to confirm.')
    fill_in 'validation', with: 'DELETE'
    click_button('Delete')
    assert page.has_content?('Agent with the name: My awesome weather bot has successfully been deleted.')
    assert_equal '/agents', current_path
    assert_equal before_count - 1, Agent.count
  end

end
