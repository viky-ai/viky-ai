require "application_system_test_case"

class AgentsNewTest < ApplicationSystemTestCase

  def go_to_agents_creation
    go_to_agents_index
    click_link('New agent')
    within(".modal") do
      assert page.has_text? 'Create a new agent'
    end
  end


  test "Navigation to agent creation" do
    go_to_agents_index
    assert page.has_text? 'New agent'
    click_link 'New agent'

    within(".modal") do
      assert page.has_field? 'Name'
      assert page.has_field? 'ID'
      assert page.has_button? 'Create'
      assert page.has_link? 'Cancel'
    end
  end


  test "Agent creation form ok with background color" do
    go_to_agents_creation

    within(".modal") do
      fill_in 'Name', with: 'Wall-e'
      fill_in 'ID', with: 'wall-e'

      first("a.background-color__red").click
      click_button 'Create'
    end

    assert page.has_text?('Your agent has been successfully created.')
    expected = [
      "My awesome weather bot",
      "T-800",
      "Wall-e",
    ]
    assert_equal expected, (all('.agent-box h2').collect {|n| n.text})
    assert_equal "Wall-e", first(".background-color-gradient__red h2").text
  end


  test "Agent creation form ok with uploaded background image" do
    go_to_agents_creation

    within(".modal") do
      fill_in 'Name', with: 'Wall-e'
      fill_in 'ID', with: 'wall-e'

      click_link("Or upload an image")
      file = File.join(Rails.root, 'test', 'fixtures', 'files', 'wall-e.jpg')

      # https://github.com/teampoltergeist/poltergeist/issues/866
      attach_file('agent_image', file).click

      click_button 'Create'
    end

    assert page.has_text?('Your agent has been successfully created.')
    expected = [
      "My awesome weather bot",
      "T-800",
      "Wall-e",
    ]
    assert_equal expected, (all('.agent-box h2').collect {|n| n.text})
    assert all('.agent-box__header').last[:style].include? "background-image"
  end


  test "Agent creation form cancel" do
    go_to_agents_creation
    click_link 'Cancel'
    assert page.has_no_text?('Create new agent')
  end


  test "Agent creation form error required" do
    go_to_agents_creation
    click_button 'Create'
    expected = [
      "Name can't be blank",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank",
    ]
    expected.each do |error|
      assert page.has_text?(error)
    end
    assert_equal 3, all('.help--error').size
  end


  test "Agent creation form error too short" do
    go_to_agents_creation
    fill_in 'Name', with: 'Wall-e'
    fill_in 'ID', with: 'A'
    click_button 'Create'
    expected = ["ID is too short (minimum is 3 characters)"]
    expected.each do |error|
      assert page.has_text?(error)
    end
    assert_equal 1, all('.help--error').size
  end


  test "No Api Token is shown in creation" do
    go_to_agents_creation
    within(".modal") do
      assert !page.has_text?("Api token")
      assert_nil first("#agent_api_token")
    end
  end

end
