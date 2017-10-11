require "application_system_test_case"

class NewAgentsTest < ApplicationSystemTestCase

  def go_to_agents_creation
    admin_login
    within(".nav") do
      click_link "Agents"
    end
    click_link('New agent')
  end

  test "Navigation to agent creation" do
    go_to_agents_index
    assert page.has_content?("New agent")
    click_link('New agent')
    within("form") do
      assert page.has_field?("Agent name")
      assert page.has_field?("Agent ID")
      assert page.has_button?("Create")
      assert page.has_link?("Cancel")
    end
  end

  test "Agent creation form ok" do
    go_to_agents_creation
    fill_in 'Agent name', with: 'Wall-e'
    fill_in 'Agent ID', with: 'wall-e'
    click_button 'Create'
    expected = [
      "My awesome weather bot",
      "Wall-e",
    ]
    assert_equal expected, all('.agents-list__item h3').collect {|n| n.text}
  end

  test "Agent creation form cancel" do
    go_to_agents_creation
    click_link 'Cancel'
    assert_equal '/agents', current_path
  end

  test "Agent creation form error required" do
    go_to_agents_creation
    click_button 'Create'
    expected = [
      "Name can't be blank",
      "Agentname is too short (minimum is 3 characters)",
      "Agentname can't be blank",
    ]
    assert_equal expected, all('.help--error').collect {|n| n.text}
  end

  test "Agent creation form error too short" do
    go_to_agents_creation
    fill_in 'Agent name', with: 'Wall-e'
    fill_in 'Agent ID', with: 'A'
    click_button 'Create'
    expected = [
      "Agentname is too short (minimum is 3 characters)",
    ]
    assert_equal expected, all('.help--error').collect {|n| n.text}
  end
end
