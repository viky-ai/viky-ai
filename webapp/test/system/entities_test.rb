require "application_system_test_case"

class EntitiesTest < ApplicationSystemTestCase

  test "Navigate to an entity" do
    admin_go_to_agents_index
    assert_text("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert_text("Entities")
    click_link "Entities"

    assert_text("weather_conditions")
    click_link "weather_conditions"
    assert_text("Entities lists / weather_conditions PUBLIC")
    assert_text("Glued")
    assert_text("Not used by any interpretation")
  end


  test "Update proximity for the entities list" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_text("Entities lists / weather_conditions PUBLIC")
    assert_text("Glued")
    assert has_link?("Configure")
    click_link "Configure"
    within(".modal") do
      assert has_text? "Edit entities list"
      all(".dropdown__trigger > .btn")[0].click
      click_link "Far"
      click_button "Update"
    end
    assert_text("Your entities list has been successfully updated.")
    assert_text("Entities lists / weather_conditions PUBLIC")
    assert_text("Far")
  end


  test "Used by button in entity details" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_dates))
    assert has_link?("Used by...")
    click_link "Used by..."
    within(".modal__main") do
      assert_text("Interpretations using weather_dates")
      click_link("weather_forecast")
    end
    assert_text("Interpretations / weather_forecast")
  end


  test "Pagination enabled or disabled" do
    entities_list = entities_lists(:weather_conditions)
    admin_go_to_entities_list_show(agents(:weather), entities_list)

    # Only 2 entities :
    # - Pagination in disabled
    # - Entities are draggable
    assert_selector(".card-list__item__draggable", count: 2)

    (1..100).each do |i|
      Entity.create!({
        terms: "term_#{i}",
        auto_solution_enabled: true,
        entities_list: entities_list
      })
    end

    # 102 entities:
    # - Pagination is enabled
    # - Entities are not draggable
    # - Add form is only displayed on first page
    visit user_agent_entities_list_path(users(:admin), agents(:weather), entities_list)

    assert_text("term_100")
    assert_no_selector(".card-list__item__draggable")
    assert_selector("ul#entities-list li", count: 20)
    assert_selector("#entities-form", count: 1)

    within ".pagination" do
      click_link "2"
    end

    assert_text("term_80")
    assert_no_selector(".card-list__item__draggable")
    assert_selector("ul#entities-list li", count: 20)
    assert_no_selector("#entities-form")
  end


  test "search" do
    entities_list = entities_lists(:weather_conditions)
    admin_go_to_entities_list_show(agents(:weather), entities_list)

    # Only 2 entities :
    # - No search
    assert_selector(".entities-search-and-page-entries form", count: 0)

    (1..100).each do |i|
      Entity.create!({
        terms: "term_#{i}",
        auto_solution_enabled: true,
        entities_list: entities_list
      })
    end

    # 102 entities:
    # - Search enabled
    visit user_agent_entities_list_path(users(:admin), agents(:weather), entities_list)

    assert_text("term_100")

    within ".entities-search-and-page-entries" do
      fill_in "search", with: "term"
      first("button").click
      assert_text("Displaying entities 1 - 20 of 100 in total")

      fill_in "search", with: "term_100"
      first("button").click
      assert_text("Displaying 1 entity")
    end
  end


  test "Create an entity and failed" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within(".entity-form") do
      click_button "Add"
    end
    assert_text("Terms can't be blank")
    assert_text("Solution can't be blank")
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
    assert_text("foggy")
    assert_text("Entity has been successfully created.")
  end


  test "Create an entity with locale" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_no_text("foggy")
    within(".entity-form") do
      fill_in "Terms", with: "foggy:en\nbrumeux"
      check("Auto solution")
      click_button "Add"
    end
    assert_text("foggy:en")
    assert_text("Entity has been successfully created.")
  end


  test "Update an entity (simple)" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert has_link?("soleil")
    within("#entities-list") do
      click_link "soleil"
      assert_text("Cancel")
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
      assert_text("Cancel")
      fill_in "Terms", with: "Canicule:xx"
      click_button "Update"
    end
    assert_text("Terms uses an unauthorized locale 'xx' for this agent")
  end


  test "Show an entity with details" do
    login_as "show_on_agent_weather@viky.ai", "BimBamBoom"
    assert_text("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert_text("Entities")
    click_link "Entities"

    assert_text("weather_conditions")
    click_link "weather_conditions"
    assert_text("Entities lists / weather_conditions PUBLIC")
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
      assert_text("Delete")
      all("a").last.click
    end

    assert_text("Entity has been successfully deleted.")
    assert_equal 1, all("#entities-list li").size
    assert_not has_link?("pluie")
    assert has_link?("soleil")
  end


  test "Delete all entities and display blankstate" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))

    assert_equal 2, all("#entities-list li").size
    assert has_link?("pluie")
    assert has_link?("soleil")

    within("#entities-list") do
      click_link "pluie"
      assert_text("Delete")
      all("a").last.click
    end

    assert_text("Entity has been successfully deleted.")
    assert_equal 1, all("#entities-list li").size
    assert_not has_link?("pluie")
    assert has_link?("soleil")

    within("#entities-list") do
      click_link "soleil"
      assert_text("Delete")
      all("a").last.click
    end

    assert_text("Entity has been successfully deleted.")
    assert_not has_link?("pluie")
    assert_not has_link?("soleil")
    assert_text("Start adding entity")
  end


  test "Entity autosolution synchronization" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within(".entity-form") do
      assert has_no_text?("brumeux")
      check("Auto solution")
      execute_script %Q{ $("#terms__new_entity").trigger("click") }
      fill_in "Terms", with: "  \nbrumeux"
      assert_text('"brumeux"')

      uncheck("Auto solution")
      fill_in "Terms", with: "foggy\nbrumeux"
      assert_text('"brumeux"')
      check("Auto solution")
      assert_text('"foggy"')
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

    within("#entities-list") do
      click_link "brumeux"
      assert_text('"condition: brumeux"')
    end
  end


  test "Export entities" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within ".header" do
      assert has_link? "Export"
    end
  end


  test "Import entities hidden when no edit right" do
    login_as "show_on_agent_weather@viky.ai", "BimBamBoom"
    visit user_agent_entities_list_path(users(:admin), agents(:weather), entities_lists(:weather_conditions))
    within ".header" do
      assert has_no_link? "Import"
    end
  end


  test "Import entities without file" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    click_link "Import"
    within(".modal") do
      assert_text "Import entities"
      assert_no_text "File must be present"
      click_button "Import"
      assert_text "File must be present"
    end
  end


  test "Import entities" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_equal 2, all("#entities-list > li").count
    click_link "Import"

    within(".modal") do
      assert_text "Import entities"
      file = File.join(Rails.root, "test", "fixtures", "files", "import_entities.csv")

      # Display import file imput in order to allow capybara attach_file
      execute_script("$('#import_file').css('opacity','1')");
      attach_file("import_file", file)
      click_button "Import"
      assert has_text? "Uploading file, please wait..."
    end
  end


  # test "Import replace entities" do
  #   admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
  #   assert_equal 2, all("#entities-list > li").count
  #   click_link "Import"
  #   within(".modal") do
  #     assert_text "Import entities"
  #     file = File.join(Rails.root, "test", "fixtures", "files", "import_entities.csv")
  #
  #     # Display import file imput in order to allow capybara attach_file
  #     execute_script("$('#import_file').css('opacity','1');")
  #     attach_file("import_file", file)
  #
  #     choose "Replace current entities"
  #     click_button "Import"
  #     assert has_text? "Uploading file, please wait..."
  #   end
  # end


  private

    def admin_go_to_entities_list_show(agent, entities_list)
      admin_login
      visit user_agent_entities_list_path(users(:admin), agent, entities_list)
      assert_text("#{entities_list.listname} PUBLIC")
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
