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
  test 'Button to delete agent is not present' do
    go_to_agents_index
    first('.dropdown__trigger > button').click
    assert !page.has_link?("Delete")
  end

  test 'Button to delete agent is (not) present' do
    # Make agent deletable
    agent = agents(:weather)
    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end

    go_to_agents_index
    first('.dropdown__trigger > button').click
    assert page.has_link?("Delete")
  end


  test 'Delete with confirmation' do
    # Make agent deletable
    agent = agents(:weather)
    agent.memberships.where.not(rights: 'all').each do |m|
      assert m.destroy
    end

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


  #
  # Share
  #
  test 'Share agent user' do
    go_to_agents_index

    first('.dropdown__trigger > button').click
    click_link 'Share'
    within(".modal") do
      click_link 'Invite collaborators'
      assert page.has_content?('Share with')
      page.execute_script "document.getElementById('input-user-search').value = 'confirmed'"
      click_button 'Invite'
    end
    assert page.has_text?('Agent weather shared with : confirmed.')
  end


  test 'Share agent empty user' do
    go_to_agents_index

    first('.dropdown__trigger > button').click
    click_link 'Share'
    within(".modal") do
      click_link 'Invite collaborators'
      assert page.has_content?('Share with')
      click_button 'Invite'
    end
    assert page.has_text?('Please select a valid user.')
  end


  test 'Share agent with multiple users at the same time' do
    go_to_agents_index

    agent_terminator_dropdown = all('.dropdown__trigger > button')[-1]
    agent_terminator_dropdown.click
    click_link 'Share'
    within(".modal") do
      click_link 'Invite collaborators'
      assert page.has_content?('Share with')
      page.execute_script "document.getElementById('input-user-search').value = 'confirmed;show_on_agent_weather'"
      click_button 'Invite'
    end
    assert page.has_text?('Agent terminator shared with : confirmed, show_on_agent_weather.')
  end

end
