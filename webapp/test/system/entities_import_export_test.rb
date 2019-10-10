require "application_system_test_case"

class EntitiesImportExportTest < ApplicationSystemTestCase

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
      assert has_text?("Import entities")
      assert has_no_text?("File must be present")
      click_button "Import"
      assert has_text?("File must be present")
    end
  end


  test "Import entities falling" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_equal 2, all("#entities-list > li").count
    click_link "Import"

    perform_enqueued_jobs do
      within(".modal") do
        assert has_text?("Import entities")
        file = build_fixture_files_path('import_entities_ko.csv')
        attach_file("import_file", file, make_visible: true)
        click_button "Import"
      end
      assert has_text?("Import failed")
    end
  end


  test "Import append entities with success" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_equal 2, all("#entities-list > li").count
    click_link "Import"

    perform_enqueued_jobs do
      within(".modal") do
        assert has_text?("Import entities")
        file = build_fixture_files_path('import_entities_ok.csv')
        attach_file("import_file", file, make_visible: true)
        click_button "Import"
      end
      assert has_text?("Processing import")
      assert has_text?("3 entities imported successfully")
    end
    # Wait redirection
    sleep 6
    assert has_css?("#entities-list > li", count: 5)
  end


  test "Import replace entities with success" do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert_equal 2, all("#entities-list > li").count
    click_link "Import"

    perform_enqueued_jobs do
      within(".modal") do
        assert has_text? "Import entities"
        file = build_fixture_files_path('import_entities_ok.csv')
        attach_file("import_file", file, make_visible: true)
        choose "Replace current entities"
        click_button "Import"
      end
      assert has_text? "Processing import"
      assert has_text? "3 entities imported successfully"
    end
    # Wait redirection
    sleep 6
    assert has_css?("#entities-list > li" ,count: 3)
  end


  private

    def admin_go_to_entities_list_show(agent, entities_list)
      admin_login
      visit user_agent_entities_list_path(users(:admin), agent, entities_list)
      assert has_text?("#{entities_list.listname} PUBLIC")
    end

end
