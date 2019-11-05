require "application_system_test_case"

class InterpretationsTest < ApplicationSystemTestCase

  test "Create an interpretation" do
    admin_go_to_agent_interpretations(agents("terminator"))
    click_link "New interpretation"
    within(".modal") do
      assert has_text?("Create a new interpretation")
      fill_in "ID", with: "sunny_day"
      fill_in "Description", with: "Questions about the next sunny day"
      click_button "Private"
      click_button "Create"
    end
    assert has_text?("Interpretation has been successfully created.")
  end


  test "Errors on interpretation creation" do
    admin_go_to_agent_interpretations(agents("terminator"))
    click_link "New interpretation"
    within(".modal") do
      assert has_text?("Create a new interpretation")
      fill_in "ID", with: ""
      fill_in "Description", with: "Questions about the next sunny day"
      click_button "Create"
      assert has_text?("ID is too short (minimum is 3 characters)")
      assert has_text?("ID can't be blank")
    end
  end


  test "Update an interpretation" do
    admin_go_to_agent_interpretations(agents("weather"))
    within "#interpretations-list-is_public" do
      first(".dropdown__trigger > button").click
      click_link "Configure"
    end

    within(".modal") do
      assert has_text?("Edit interpretation")
      fill_in "ID", with: "sunny_day"
      fill_in "Description", with: "Questions about the next sunny day"
      click_button "Update"
    end
    assert has_text?("Your interpretation has been successfully updated.")
  end


  test "Delete an interpretation" do
    agent = agents(:weather)

    admin_go_to_agent_interpretations(agent)
    within "#interpretations-list-is_public" do
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
    assert has_text?("Interpretation with the name: weather_forecast has successfully been deleted.")

    assert_equal user_agent_interpretations_path(agent.owner, agent), current_path
  end


  test "Reorganize interpretations" do
    interpretation = Interpretation.new(interpretation_name: "test")
    interpretation.agent = agents(:weather)
    assert interpretation.save

    admin_go_to_agent_interpretations(agents("weather"))
    expected = ["test", "weather_forecast", "weather_question"]
    assert_equal expected, all(".card-list__item__name").collect(&:text)

    assert has_css?(".card-list__item__draggable", count: 3)

    first_item = all(".card-list__item__draggable").first
    last_item  = all(".card-list__item__draggable").last
    first_item.drag_to(last_item)

    expected = ["weather_forecast", "weather_question", "test"]
    assert_equal expected, all(".card-list__item__name").collect(&:text)

    # Is persisted ?
    admin_go_to_agent_interpretations(agents("weather"))
    assert_equal expected, all(".card-list__item__name").collect(&:text)
  end


  test "Add locale to an interpretation" do
    admin_go_to_agent_interpretations(agents("terminator"))
    click_link "terminator_find"

    assert has_text?("+")
    assert has_no_text?("fr")
    click_link "+"
    within(".modal") do
      assert has_text?("Choose a language")
      click_link("fr (French)")
    end
    assert has_text?("fr 0")
  end


  test "locale navigation persistence" do
    admin_go_to_agent_interpretations(agents("terminator"))
    click_link "terminator_find"
    assert has_text?("+")

    # Add fr locale
    click_link "+"
    within(".modal") do
      assert has_text?("Choose a language")
      click_link("fr (French)")
    end
    assert has_text?("fr 0")
    assert_equal "fr 0", first("li[data-locale] a.current").text

    # Add es locale
    click_link "+"
    within(".modal") do
      assert has_text?("Choose a language")
      click_link("es (Spanish)")
    end
    assert has_text?("es 0")
    assert_equal "es 0", first("li[data-locale] a.current").text

    # Leaves and comes back
    within ".header__breadcrumb" do
      click_link "Interpretations"
    end
    click_link "terminator_find"
    assert has_text?("+")
    assert_equal "es 0", first("li[data-locale] a.current").text

    # Switch to fr, leaves and comes back
    click_link "fr 0"
    within ".header__breadcrumb" do
      click_link "Interpretations"
    end
    click_link "terminator_find"
    assert has_text?("+")
    assert_equal "fr 0", first("li[data-locale] a.current").text
  end


  test "Move interpretation to another agent" do
    interpretation = interpretations(:weather_question)
    interpretation.visibility = Interpretation.visibilities[:is_private]
    assert interpretation.save

    admin_go_to_agent_interpretations(agents("weather"))
    within "#interpretations-list-is_private" do
      first(".dropdown__trigger > button").click
      assert has_no_link?("Move to T-800")
      click_link "Move to..."
    end

    within(".modal") do
      click_link "T-800"
    end

    assert has_text?("interpretation weather_question moved to agent T-800")
    assert has_link?("T-800")

    within "#interpretations-list-is_public" do
      first(".dropdown__trigger > button").click
      assert has_link?("Move to T-800")
    end

    admin_go_to_agent_interpretations(agents("terminator"))
    within "#interpretations-list-is_private" do
      assert has_text?("weather_question")
      first(".dropdown__trigger > button").click
      assert has_no_link?("Move to T-800")
    end
  end


  test "Filter favorite agent select" do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: "edit")
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    admin_go_to_agent_interpretations(agents("terminator"))
    within "#interpretations-list-is_public" do
      first(".dropdown__trigger > button").click
      click_link "Move to"
    end

    within(".modal") do
      click_button "Favorites"
      assert has_no_text?("My awesome weather bot\nadmin/weather")
      assert has_text?("Weather bot\nconfirmed/weather")
    end
  end


  test "Filter query agent dependency" do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: "edit")
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    admin_go_to_agent_interpretations(agents("terminator"))
    within "#interpretations-list-is_public" do
      first(".dropdown__trigger > button").click
      click_link "Move to"
    end

    within(".modal") do
      fill_in "search_query", with: "awesome"
      click_button "#search"
      assert has_text?("My awesome weather bot\nadmin/weather")
      assert has_no_text?("Weather bot\nconfirmed/weather")
    end
  end

  test "Used by menu" do
    admin_go_to_agent_interpretations(agents("terminator"))

    within "#interpretations-list-is_public" do
      assert first("li").has_no_link?("Used by...")
    end

    within "#interpretations-list-is_private" do
      assert first("li").has_link?("Used by...")
      # click link and check
      click_link "Used by..."
    end

    assert has_text?("Interpretations using simple_where")
    within ".modal" do
      assert has_link?("terminator_find")
      click_link("terminator_find")
    end

    assert current_url.include?("/agents/admin/terminator/interpretations#smooth-scroll-to-interpretation-#{interpretations(:terminator_find).id}")
  end

  private

    def admin_go_to_agent_interpretations(agent)
      admin_login
      go_to_agent_interpretations(agent)
    end
end
