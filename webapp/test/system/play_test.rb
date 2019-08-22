require "application_system_test_case"

class PlayTest < ApplicationSystemTestCase

  test "Display blank slate" do
    go_to_play_ui
  end


  test "Agents selection" do
    go_to_play_ui

    select_2_agents

    within "aside" do
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


  test "Agent selection search" do
    go_to_play_ui

    click_link 'Choose agents'
    within ".modal" do
      assert page.has_text?("Choose playground's agents")
      assert page.has_text?("0 selected agent")

      expected = ["admin/weather", "admin/terminator"]
      assert_equal expected, all('ul.agent-compact-list li h6').collect(&:text)

      all('ul.agent-compact-list li')[0].click
      assert page.has_text?("1 selected agent")

      click_button "Selected"
      sleep 0.25
      assert_equal ["admin/weather"], all('ul.agent-compact-list li h6').collect(&:text)

      click_button "Unselected"
      sleep 0.25
      assert_equal ["admin/terminator"], all('ul.agent-compact-list li h6').collect(&:text)

      first('.btn-group button').click
      sleep 0.25
      assert_equal expected, all('ul.agent-compact-list li h6').collect(&:text)

      fill_in 'search[query]', with: 'terminator'
      first("#search-button").click
      sleep 0.25
      assert_equal ["admin/terminator"], all('ul.agent-compact-list li h6').collect(&:text)

      fill_in 'search[query]', with: 'Missing agent'
      first("#search-button").click
      sleep 0.25
      assert_equal "No agent found.", first('.modal__main__chooser__no-search-result').text
    end
  end


  test "Form validations" do
    go_to_play_ui
    select_2_agents

    within ".play-main__form" do
      click_button "Interpret"
      assert page.has_text?("Text can't be blank")
    end
  end


  test "Error on NLP Call" do
    go_to_play_ui
    select_2_agents

    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns({
      status: 503,
      body: {
        errors: ["First error", "Second error"]
      }
    })

    within ".play-main__form" do
      fill_in 'play_interpreter_text', with: 'Hello NLP'
      click_button "Interpret"
    end

    assert page.has_text?("Sorry, an error occurred")
    assert page.has_text?("Error 503 — First error, Second error.")
  end


  test "Success on NLP Call" do
    go_to_play_ui
    select_2_agents

    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns({
      status: 200,
      body: {
        'interpretations' => [
          {
            "id" => intents(:weather_forecast).id,
            "slug" => "admin/weather/weather_forecast",
            "name" => "weather_forecast",
            "score" => 1.0,
            "start_position" => 6,
            "end_position"=> 9,
            "solution"=> {}
          }
        ]
      }
    })

    within ".play-main__form" do
      fill_in 'play_interpreter_text', with: 'Hello NLP'
      click_button "Interpret"
    end

    assert page.has_text?("1 interpretation found")
    assert_equal "NLP", first('.play-main__result__wrapper .highlight').text

    first('.play-main__result__wrapper .highlight').click

    expected = "INTERPRETATION admin/weather/weather_forecast SOLUTION { }"
    assert_equal expected, first('.play-main__result__wrapper .highlight-pop').text

    # Validate Play UI persistence
    within ".h-nav" do
      click_link "Agents"
    end
    within("header") do
      assert page.has_text?("Agents")
    end
    within ".h-nav" do
      click_link "Play"
    end
    assert page.has_text?("Hello NLP")

    click_link "Reset"
    assert page.has_no_text?("Hello NLP")
  end


  def select_2_agents
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
    end
  end

  def go_to_play_ui
    admin_login
    within ".h-nav" do
      click_link "Play"
    end
    assert page.has_text?("Welcome to playground!")
  end

end
