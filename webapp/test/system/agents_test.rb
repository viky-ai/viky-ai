require "application_system_test_case"

class AgentsTest < ApplicationSystemTestCase

  test 'Navigation to agents index' do
    go_to_agents_index
    assert page.has_text?("Agents")
    assert_equal "My awesome weather bot", first('.agent-box h2').text
  end


  test 'blank slate' do
    Agent.delete_all
    go_to_agents_index
    assert page.has_text?("Create your first agent")

    click_link 'New agent'

    within(".modal") do
      assert page.has_field? 'Name'
      assert page.has_field? 'ID'
      assert page.has_button? 'Create'
    end
  end


  #
  # Delete
  #
  test 'Button to delete agent is present' do
    go_to_agents_index
    first('.dropdown__trigger > button').click
    assert page.has_link?("Delete")
  end


  test 'Delete with confirmation' do
    before_count = Agent.count
    go_to_agents_index

    first('.dropdown__trigger > button').click
    click_link 'Delete'

    assert page.has_text?('Are you sure?')
    click_button('Delete')
    assert page.has_text?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', with: 'dElEtE'
    click_button('Delete')
    assert page.has_text?('Please enter the text exactly as it is displayed to confirm.')

    fill_in 'validation', with: 'DELETE'
    click_button('Delete')
    assert page.has_text?('Agent with the name: My awesome weather bot has successfully been deleted.')

    assert_equal '/agents', current_path
    assert_equal before_count - 1, Agent.count
  end


  #
  # Configure
  #
  test 'Configure from index' do
    go_to_agents_index
    first('.dropdown__trigger > button').click
    click_link 'Configure'
    assert page.has_text?('Configure agent')
    fill_in 'Name', with: ''
    click_button 'Update'

    expected = ["Name can't be blank"]
    expected.each do |error|
      assert page.has_text?(error)
    end
    assert_equal 1, all('.help--error').size

    fill_in 'Name', with: 'My new updated agent'
    click_button 'Update'
    assert page.has_text?('Your agent has been successfully updated.')
    assert page.has_text?('My new updated agent')
  end


  test 'Cancel configure from index' do
    go_to_agents_index
    first('.dropdown__trigger > button').click
    click_link 'Configure'
    assert page.has_text?('Configure agent')
    click_link 'Cancel'
    assert page.has_no_text?('Configure agent')
    assert_equal '/', current_path
  end


  #
  # Search
  #
  test 'Agents can be found by name' do
    go_to_agents_index
    fill_in 'search_query', with: '800'
    click_button '#search'
    assert page.has_content?('T-800')
    assert page.has_no_content?('My awesome weather bot')
    assert_equal '/agents', current_path
  end


  test 'Agents can be found by agentname' do
    go_to_agents_index
    fill_in 'search_query', with: 'inator'
    click_button '#search'
    assert page.has_content?('T-800')
    assert page.has_no_content?('My awesome weather bot')
    assert_equal '/agents', current_path
  end


  test 'Empty search agent' do
    go_to_agents_index
    fill_in 'search_query', with: 'inator'
    click_button '#search'
    assert page.has_content?('T-800')
    assert page.has_no_content?('My awesome weather bot')
    fill_in 'search_query', with: ''
    click_button '#search'
    assert page.has_content?('T-800')
    assert page.has_content?('My awesome weather bot')
    assert_equal '/agents', current_path
  end


  test "Api Token is shown in edit" do
    go_to_agents_index
    first('.dropdown__trigger > button').click
    click_link 'Configure'
    assert page.has_text?('Configure agent')

    assert page.has_text?('Api token')
    assert_not_nil find("#agent_api_token")[:readonly]
    prev_value = find("#agent_api_token").value

    all("a", text: "Generate").first.click
    click_button 'Update'

    assert_not page.has_text?(prev_value)
    first('.dropdown__trigger > button').click
    click_link 'Configure'
    after_value = find("#agent_api_token").value

    assert_not_equal prev_value, after_value
  end

end
