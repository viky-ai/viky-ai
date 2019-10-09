require "application_system_test_case"

class EntitiesTest < ApplicationSystemTestCase

  test "Navigate to an entity" do
    admin_go_to_agents_index
    assert has_text?("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Entities")
    click_link "Entities"

    assert has_text?("weather_conditions")
    click_link "weather_conditions"
    assert has_text?("Entities lists / weather_conditions PUBLIC")
    assert has_text?("Glued")
    assert has_text?("Not used by any interpretation")
  end


  test "Update proximity for the entities list" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_text?("Entities lists / weather_conditions PUBLIC")
    assert has_text?("Glued")
    assert has_link?("Configure")
    click_link "Configure"
    within(".modal") do
      assert has_text? "Edit entities list"
      all(".dropdown__trigger > .btn")[0].click
      click_link "Far"
      click_button "Update"
    end
    assert has_text?("Your entities list has been successfully updated.")
    assert has_text?("Entities lists / weather_conditions PUBLIC")
    assert has_text?("Far")
  end


  test "Used by button in entity details" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_dates))
    assert has_link?("Used by...")
    click_link "Used by..."
    within(".modal__main") do
      assert has_text?("Interpretations using weather_dates")
      click_link("weather_forecast")
    end
    assert has_text?("Interpretations / weather_forecast")
  end


  test "Pagination enabled or disabled" do
    entities_list = entities_lists(:weather_conditions)
    admin_go_to_entities_list_show(agents(:weather), entities_list)

    # Only 2 entities :
    # - Pagination in disabled
    # - Entities are draggable
    assert has_css?(".card-list__item__draggable", count: 2)

    (2..101).each do |i|
      Entity.create!({
        terms: "term_#{i}",
        auto_solution_enabled: true,
        entities_list: entities_list,
        position: i
      })
    end

    # 102 entities:
    # - Pagination is enabled
    # - Entities are not draggable
    # - Add form is only displayed on first page
    visit user_agent_entities_list_path(users(:admin), agents(:weather), entities_list)

    assert has_text?("term_100")
    assert has_no_css?(".card-list__item__draggable")
    assert has_css?("ul#entities-list li", count: 20)
    assert has_css?("#entities-form", count: 1)

    within ".pagination" do
      click_link "2"
    end

    assert has_text?("term_80")
    assert has_no_css?(".card-list__item__draggable")
    assert has_css?("ul#entities-list li", count: 20)
    assert has_no_css?("#entities-form")
  end


  test "search" do
    entities_list = entities_lists(:weather_conditions)
    admin_go_to_entities_list_show(agents(:weather), entities_list)

    # Only 2 entities :
    # - No search
    assert has_css?(".entities-search-and-page-entries form", count: 0)

    (2..101).each do |i|
      Entity.create!({
        terms: "term_#{i}",
        auto_solution_enabled: true,
        entities_list: entities_list,
        position: i
      })
    end

    # 102 entities:
    # - Search enabled
    visit user_agent_entities_list_path(users(:admin), agents(:weather), entities_list)

    assert has_text?("term_100")

    within ".entities-search-and-page-entries" do
      fill_in "search", with: "term"
      first("button").click
      assert has_text?("Displaying entities 1 - 20 of 100 in total")

      fill_in "search", with: "term_100"
      first("button").click
      assert has_text?("Displaying 1 entity")
    end
  end


  test "Create an entity and failed" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within(".entity-form") do
      click_button "Add"
    end
    assert has_text?("Terms can't be blank")
    assert has_text?("Solution can't be blank")
  end


  test "Create an entity and succeed" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_no_text?("foggy")
    within(".entity-form") do
      fill_in "Terms", with: "foggy\nbrumeux"
      uncheck("Auto solution")
      fill_in_editor_field "condition: brumeux"
      click_button "Add"
    end
    assert has_text?("Entity has been successfully created.")
    assert has_text?("foggy")
  end


  test "Create an entity with locale" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_no_text?("foggy")
    within(".entity-form") do
      fill_in "Terms", with: "foggy:en\nbrumeux"
      check("Auto solution")
      click_button "Add"
    end
    assert has_text?("Entity has been successfully created.")
    assert has_text?("foggy:en")
  end


  test "Update an entity (simple)" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_link?("soleil")
    within("#entities-list") do
      click_link "soleil"
      assert has_text?("Cancel")
      fill_in "Terms", with: "Canicule"
      check("Auto solution")
      click_button "Update"
    end
    assert has_link?("Canicule")
  end


  test "Update an entity with wrong locale" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_link?("soleil")
    within("#entities-list") do
      click_link "soleil"
      assert has_text?("Cancel")
      fill_in "Terms", with: "Canicule:xx"
      click_button "Update"
    end
    assert has_text?("Terms uses an unauthorized locale 'xx' for this agent")
  end


  test "Show an entity with details" do
    login_as "show_on_agent_weather@viky.ai", "BimBamBoom"
    assert has_text?("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Entities")
    click_link "Entities"

    assert has_text?("weather_conditions")
    click_link "weather_conditions"
    assert has_text?("Entities lists / weather_conditions PUBLIC")
    within("#entities-list") do
      click_link "soleil"
      assert has_no_button?("Update")
    end
  end


  test "Delete an entity" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_equal 2, all("#entities-list li").size
    assert has_link?("pluie")
    assert has_link?("soleil")

    within("#entities-list") do
      click_link "pluie"
      assert has_text?("Delete")
      all("a").last.click
    end

    assert has_text?("Entity has been successfully deleted.")
    assert has_css?("#entities-list li", count: 1)
    assert has_no_link?("pluie")
    assert has_link?("soleil")
  end


  test "Delete all entities and display blankstate" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))

    assert has_css?("#entities-list li", count: 2)
    assert has_link?("pluie")
    assert has_link?("soleil")

    within("#entities-list") do
      click_link "pluie"
      assert has_text?("Delete")
      all("a").last.click
    end

    assert has_text?("Entity has been successfully deleted.")
    assert has_css?("#entities-list li", count: 1)
    assert has_no_link?("pluie")
    assert has_link?("soleil")

    within("#entities-list") do
      click_link "soleil"
      assert has_text?("Delete")
      all("a").last.click
    end

    assert has_text?("Entity has been successfully deleted.")
    assert has_no_link?("pluie")
    assert has_no_link?("soleil")
    assert has_text?("Start adding entity")
  end


  test "Entity autosolution synchronization" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within(".entity-form") do
      assert has_no_text?("brumeux")
      check("Auto solution")
      execute_script %Q{ $("#terms__new_entity").trigger("click") }
      fill_in "Terms", with: "  \nbrumeux"
      assert has_text?('"brumeux"')

      uncheck("Auto solution")
      fill_in "Terms", with: "foggy\nbrumeux"
      assert has_text?('"brumeux"')
      check("Auto solution")
      assert has_text?('"foggy"')
    end
  end


  test "Keep solution after unchecking checkbox" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))

    within(".entity-form") do
      uncheck("Auto solution")
      execute_script %Q{ $("#terms__new_entity").trigger("click") }
      fill_in "Terms", with: "brumeux"
      fill_in_editor_field '"condition: brumeux"'
      click_button "Add"
    end

    assert has_text? "Entity has been successfully created."

    within("#entities-list") do
      click_link "brumeux"
      assert has_css?(".entity-form")
      assert has_text?('"condition: brumeux"')
    end
  end


  private

    def admin_go_to_entities_list_show(agent, entities_list)
      admin_login
      visit user_agent_entities_list_path(users(:admin), agent, entities_list)
      assert has_text?("#{entities_list.listname} PUBLIC")
    end

    def fill_in_editor_field(text)
      within ".CodeMirror" do
        # Click makes CodeMirror element active:
        current_scope.click
        # Find the hidden textarea:
        field = current_scope.find("textarea", visible: false)
        # Mimic user typing the text:
        field.send_keys text
      end
    end
end
