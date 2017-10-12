require "application_system_test_case"

class NewAgentsTest < ApplicationSystemTestCase


  def go_to_agents_creation
    admin_login
    within(".nav") do
      click_link 'Agents'
    end
    click_link('New agent')
    assert page.has_text? 'Create new agent'
  end


  test "Navigation to agent creation" do
    go_to_agents_index
    assert page.has_text? 'New agent'
    click_link 'New agent'

    within(".modal") do
      assert page.has_field? 'Agent name'
      assert page.has_field? 'Agent ID'
      assert page.has_button? 'Create'
      assert page.has_link? 'Cancel'
    end
  end


  test "Agent creation form ok" do
    go_to_agents_creation
    fill_in 'Agent name', with: 'Wall-e'
    fill_in 'Agent ID', with: 'wall-e'
    click_button 'Create'

    assert page.has_text?('Your agent has been succefully created.')
    expected = [
      "My awesome weather bot",
      "Wall-e",
    ]
    assert_equal expected, all('.agent-box h2').collect {|n| n.text}
  end


  test "Agent creation form cancel" do
    go_to_agents_creation
    assert page.has_text?('Create new agent')
    click_link 'Cancel'
    assert page.has_no_text?('Create new agent')
  end


  test "Agent creation form error required" do
    go_to_agents_creation
    click_button 'Create'
    expected = [
      "Name can't be blank",
      "Agentname is too short (minimum is 3 characters)",
      "Agentname can't be blank",
    ]
    expected.each do |error|
      assert page.has_text?(error)
    end
    assert_equal 3, all('.help--error').size
  end


  test "Agent creation form error too short" do
    go_to_agents_creation
    fill_in 'Agent name', with: 'Wall-e'
    fill_in 'Agent ID', with: 'A'
    click_button 'Create'
    expected = ["Agentname is too short (minimum is 3 characters)"]
    expected.each do |error|
      assert page.has_text?(error)
    end
    assert_equal 1, all('.help--error').size
  end

end
