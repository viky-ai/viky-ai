require "application_system_test_case"

class PlayTest < ApplicationSystemTestCase

  test "Display blank slate" do
    go_to_play_ui
  end


  test "Agents selection" do
    go_to_play_ui

    # Add 2 agents
    click_link 'Choose agents'
    within ".modal" do
      assert page.has_text?("Choose playground's agents")
      assert page.has_text?("0 selected agent")

      all('ul.agent-compact-list li')[0].click
      assert page.has_text?("1 selected agent")

      all('ul.agent-compact-list li')[1].click
      assert page.has_text?("2 selected agents")

      click_button "Let's go"
    end

    assert_modal_is_close

    within "aside" do
      expected = [
        "My awesome weather bot admin/weather",
        "PUBLIC T-800 admin/terminator"
      ]
      assert_equal expected, all('ul li').collect(&:text)
      expected = ["My awesome weather bot admin/weather"]
      assert_equal expected, all('ul li.current').collect(&:text)
      click_link "Edit"
    end

    # Remove 1 agent
    within ".modal" do
      assert page.has_text?("2 selected agent")
      first('ul.agent-compact-list li').click
      assert page.has_text?("1 selected agent")
      click_button "Let's go"
    end

    assert_modal_is_close

    within "aside" do
      expected = [
        "PUBLIC T-800 admin/terminator"
      ]
      assert_equal expected, all('ul li').collect(&:text)
      click_link "Edit"
    end

    # Return to blank slate
    within ".modal" do
      assert page.has_text?("1 selected agent")
      all('ul.agent-compact-list li')[1].click
      assert page.has_text?("0 selected agent")
      click_button "Let's go"
    end

    assert_modal_is_close
    assert page.has_text?("Welcome to playground!")
  end


  def go_to_play_ui
    admin_login
    within ".h-nav" do
      click_link "Play"
    end
    assert page.has_text?("Welcome to playground!")
  end

end
