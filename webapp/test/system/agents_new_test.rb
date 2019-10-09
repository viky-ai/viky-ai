require "application_system_test_case"

class AgentsNewTest < ApplicationSystemTestCase

  def go_to_agents_creation
    admin_go_to_agents_index
    click_link("New agent")
    within(".modal") do
      assert has_text?("Create a new agent")
    end
  end


  test "Navigation to agent creation" do
    admin_go_to_agents_index
    assert has_text?("New agent")
    click_link "New agent"

    within(".modal") do
      assert has_field?("Name")
      assert has_field?("ID")
      assert has_button?("Create")
      assert has_button?("Cancel")
    end
  end


  test "Agent creation form ok with background color" do
    go_to_agents_creation
    within(".modal") do
      fill_in "Name", with: "Wall-e"
      fill_in "ID", with: "wall-e"

      first("button.background-color__red").click
      click_button "Create"
    end

    assert has_text?("Your agent has been successfully created.")
    assert_equal user_agent_path(users(:admin), "wall-e"), current_path
    expected = [
      "My awesome weather bot",
      "T-800",
      "Wall-e",
    ]
    admin_go_to_agents_index
    assert_equal expected, (all(".agent-box h2").collect {|n| n.text})

    expected = [
      "agent-box__header background-color-gradient__black",
      "agent-box__header background-color-gradient__black",
      "agent-box__header background-color-gradient__red"
    ]
    assert_equal expected, all(".agent-box__header").collect {|n| n["class"]}
  end


  test "Agent creation form ok with uploaded background image" do
    go_to_agents_creation
    within(".modal") do
      fill_in "Name", with: "Wall-e"
      fill_in "ID", with: "wall-e"
      click_button("Or upload an image")
      file = build_fixture_files_path('wall-e.jpg')
      attach_file("agent_image", file, make_visible: true)
      click_button "Create"
    end

    assert has_text?("Your agent has been successfully created.")
    expected = [
      "My awesome weather bot",
      "T-800",
      "Wall-e",
    ]
    admin_go_to_agents_index
    assert_equal expected, (all(".agent-box h2").collect {|n| n.text})
    assert all(".agent-box__header").last[:style].include? "background-image"
  end


  test "Agent creation public" do
    go_to_agents_creation
    within(".modal") do
      fill_in "Name", with: "Wall-e"
      fill_in "ID", with: "wall-e"
      click_button "Public"

      first("button.background-color__red").click
      click_button "Create"
    end

    assert has_text?("Your agent has been successfully created.")
    expected = [
      "My awesome weather bot",
      "T-800",
      "Wall-e",
    ]
    admin_go_to_agents_index
    assert_equal expected, (all(".agent-box h2").collect {|n| n.text})
    expected = [
      "agent-box__header background-color-gradient__black",
      "agent-box__header background-color-gradient__black",
      "agent-box__header background-color-gradient__red"
    ]
    assert_equal expected,  all(".agent-box__header").collect {|n| n["class"]}
    assert_equal "PUBLIC", first(".background-color-gradient__red span").text
  end


  test "Agent creation with default locales then update" do
    go_to_agents_creation
    within(".modal") do
      fill_in "Name", with: "Locales test"
      fill_in "ID", with: "locales-test"
      click_button "Public"
      first("button.background-color__red").click
      click_button "Create"
    end
    assert has_text?("Your agent has been successfully created.")
    assert_equal ["*", "en", "fr"], Agent.find_by_name("Locales test").ordered_locales
    click_link "Configure"
    within(".modal") do
      uncheck("fr (French)")
      click_button "Update"
    end
    assert has_text?("Your agent has been successfully updated.")
    assert_equal ["*", "en"], Agent.find_by_name("Locales test").ordered_locales
  end


  test "Agent creation form cancel" do
    go_to_agents_creation
    click_button "Cancel"
    assert has_no_text?("Create new agent")
  end


  test "Agent creation form error required" do
    go_to_agents_creation
    click_button "Create"
    expected = [
      "Name can't be blank",
      "ID is too short (minimum is 3 characters)",
      "ID can't be blank",
    ]
    expected.each do |error|
      assert has_text?(error)
    end
    assert has_css?(".help--error", count: 3)
  end


  test "Agent creation form error too short" do
    go_to_agents_creation
    fill_in "Name", with: "Wall-e"
    fill_in "ID", with: "A"
    click_button "Create"
    expected = ["ID is too short (minimum is 3 characters)"]
    expected.each do |error|
      assert has_text?(error)
    end
    assert has_css?(".help--error", count: 1)
  end


  test "No Api Token is shown in creation" do
    go_to_agents_creation
    within(".modal") do
      assert has_no_text?("Api token")
      assert has_no_css?("#agent_api_token")
    end
  end

end
