require "application_system_test_case"

class BotsTest < ApplicationSystemTestCase

  test "Navigate to bots list" do
    admin_go_to_agents_index
    assert has_text?("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Bots")
    click_link "Bots"

    assert has_text?("Bots list")
    assert has_text?("https://myweatherbot.com/api/")
  end


  test "Create a bot and failed" do
    go_to_agent_bot_lists(agents(:weather))
    click_link "New bot"
    within(".bot-form") do
      fill_in "Name", with: "My new bot"
      fill_in "Endpoint", with: ""
      click_button "Create"
    end
    assert has_text?("Endpoint can't be blank")
  end


  test "Create a bot and succeed" do
    go_to_agent_bot_lists(agents(:weather))
    click_link "New bot"
    within(".bot-form") do
      fill_in "Name", with: "My new bot"
      fill_in "Endpoint", with: "https://www.my-new-bot.com/"
      click_button "Create"
    end
    assert has_text?("My new bot WIP")
    assert has_text?("https://www.my-new-bot.com/")
    assert has_text?("Your bot has been successfully created.")
  end


  test "Update a bot and failed" do
    go_to_agent_bot_lists(agents(:weather))

    within ".bots-list" do
      first(".dropdown__trigger > button").click
      click_link "Configure"
    end

    within(".bot-form") do
      fill_in "Name", with: ""
      click_button "Update"
    end
    assert has_text?("Name can't be blank")
  end


  test "Update a bot and succeed" do
    go_to_agent_bot_lists(agents(:weather))

    within ".bots-list" do
      first(".dropdown__trigger > button").click
      click_link "Configure"
    end

    within(".bot-form") do
      fill_in "Name", with: "Weather bot (updated)"
      fill_in "Endpoint", with: "https://myweatherbot.com/api/updated/"
      click_button "No"
      click_button "Update"
    end

    assert has_text?("Weather bot (updated)")
    assert has_no_text?("WIP")
    assert has_text?("https://myweatherbot.com/api/updated/")
    assert has_text?("Your bot has been successfully updated.")
  end


  test "Delete an entities list" do
    go_to_agent_bot_lists(agents(:weather))

    within ".bots-list" do
      first(".dropdown__trigger > button").click
      click_link "Delete"
    end

    within(".modal") do
      assert has_text?("Are you sure?")
      click_button("Delete")
      assert has_text?("Please enter the text exactly as it is displayed to confirm.")

      fill_in "validation", with: "DELETE"
      click_button("Delete")
    end
    assert has_text?("Your bot: Weather has successfully been deleted.")
    assert has_text?('No bot entered for the "weather" agent.')

    agent = agents(:weather)
    assert_equal user_agent_bots_path(agent.owner, agent), current_path
  end


  private

    def go_to_agent_bot_lists(agent)
      admin_login
      visit user_agent_bots_path(agent.owner, agent)
      within(".agent-header__content__nav .current") do
        assert has_text?("Bots")
      end
    end

end
