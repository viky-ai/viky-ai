require "application_system_test_case"

class EntitiesListsTest < ApplicationSystemTestCase

  test "Create an entities list" do
    admin_go_to_agent_entities_lists(agents("terminator"))
    click_link "New entities list"
    within(".modal") do
      assert has_text?("Create a new entities list")
      fill_in "ID", with: "towns"
      fill_in "Description", with: "List every towns in the world !"
      click_button "Private"
      all(".dropdown__trigger > .btn")[0].click
      click_link "Very close"
      click_button "Create"
    end
    assert has_text?("Entities list has been successfully created.")
  end


  test "Errors on entities list creation" do
    admin_go_to_agent_entities_lists(agents("terminator"))
    click_link "New entities list"
    within(".modal") do
      assert has_text?("Create a new entities list")
      fill_in "ID", with: ""
      fill_in "Description", with: "List every towns in the world !"
      all(".dropdown__trigger > .btn")[0].click
      click_link "Very close"
      click_button "Create"
      assert has_text?("ID is too short (minimum is 3 characters)")
      assert has_text?("ID can't be blank")
    end
  end


  test "Update an entities list" do
    admin_go_to_agent_entities_lists(agents("weather"))
    within "#entities_lists-list-is_public" do
      first(".dropdown__trigger > button").click
      click_link "Configure"
    end

    within(".modal") do
      assert has_text?("Edit entities list")
      fill_in "ID", with: "countries"
      fill_in "Description", with: "Every countries"
      all(".dropdown__trigger > .btn")[0].click
      click_link "Far"
      click_button "Update"
    end
    assert has_text?("Your entities list has been successfully updated.")
  end


  test "Delete an entities list" do
    agent = agents(:weather)
    admin_go_to_agent_entities_lists(agent)
    within "#entities_lists-list-is_public" do
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
    assert has_text?("Entities list with the name: weather_conditions has successfully been deleted.")

    assert_equal user_agent_entities_lists_path(agent.owner, agent), current_path
  end


  test "Reorganize entities lists" do
    entities_list = EntitiesList.new(listname: "test", agent: agents(:weather))
    assert entities_list.save
    admin_go_to_agent_entities_lists(agents("weather"))
    assert has_link?("New entities list")

    expected = ["test", "weather_conditions", "weather_dates"]
    assert_equal expected, all(".card-list__item__name").collect(&:text)

    first_item = all(".card-list__item__draggable").first
    last_item  = all(".card-list__item__draggable").last
    first_item.drag_to(last_item)

    expected = ["weather_conditions", "weather_dates", "test"]
    assert_equal expected, all(".card-list__item__name").collect(&:text)

    # Is persisted ?
    admin_go_to_agent_entities_lists(agents("weather"))
    assert_equal expected, all(".card-list__item__name").collect(&:text)
  end


  test "Move entities list to another agent" do
    assert EntitiesList.create(listname: "Other list", agent: agents(:terminator))

    admin_go_to_agent_entities_lists(agents("terminator"))
    within "#entities_lists-list-is_private" do
      first(".dropdown__trigger > button").click
      assert has_no_link?("Move to My awesome weather bot")
      click_link "Move to"
    end

    within(".modal") do
      click_link "My awesome weather bot"
    end

    assert has_text?("Entities list terminator_targets moved to agent My awesome weather bot")
    assert has_link?("My awesome weather bot")

    within "#entities_lists-list-is_public" do
      first(".dropdown__trigger > button").click
      assert has_link?("Move to My awesome weather bot")
    end

    admin_go_to_agent_entities_lists(agents("weather"))
    within "#entities_lists-list-is_private" do
      assert has_text?("terminator_targets")
      first(".dropdown__trigger > button").click
      assert has_no_link?("Move to My awesome weather bot")
    end
  end


  test "Filter favorite agent select" do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: "edit")
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    admin_go_to_agent_entities_lists(agents("terminator"))
    within "#entities_lists-list-is_private" do
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

    admin_go_to_agent_entities_lists(agents("terminator"))
    within "#entities_lists-list-is_private" do
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


  test "aliased interpretations" do
    admin_go_to_agent_entities_lists(agents("weather"))

    within "#entities_lists-list-is_public" do
      assert all("li").first.has_no_link?("Used by...")
      assert all("li").last.has_link?("Used by...")
      click_link("Used by...")
    end

    within ".modal" do
      assert has_text?("Interpretations using weather_dates")
      assert has_link?("weather_forecast")
      click_link("weather_forecast")
    end

    expected = "/agents/admin/weather/interpretations#smooth-scroll-to-interpretation-#{interpretations(:weather_forecast).id}"
    assert current_url.include?(expected)

    assert_equal 1, all('.highlight').size
    assert_equal "weather_forecast", first('.highlight .card-list__item__name').text
  end


  private

  def admin_go_to_agent_entities_lists(agent)
    admin_login
    go_to_agent_entities_lists(agent)
  end

end
