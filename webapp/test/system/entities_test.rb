require 'application_system_test_case'

class EntitiesTest < ApplicationSystemTestCase

  test 'Navigate to an entity' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('Entities lists')
    click_link 'Entities lists'

    assert page.has_text?('weather_conditions')
    click_link 'weather_conditions'
    assert page.has_text?('Entities lists / weather_conditions PUBLIC')
  end


  test 'Create an entity' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_no_text?('foggy')
    within('.entity-form') do
      fill_in 'Terms', with: "foggy\nbrumeux"
      uncheck('Auto solution')
      fill_in_editor_field 'condition: brumeux'
      click_button 'Add'
    end
    assert page.has_text?('foggy')
  end


  test 'Create an entity with locale' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_no_text?('foggy')
    within('.entity-form') do
      fill_in 'Terms', with: "foggy:en\nbrumeux"
      check('Auto solution')
      click_button 'Add'
    end
    assert page.has_text?('foggy')
  end


  test 'Update an entity (simple)' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_link?('soleil')
    within('#entities-list') do
      click_link 'soleil'
      assert page.has_text?('Cancel')
      fill_in 'Terms', with: "Canicule"
      check('Auto solution')
      click_button 'Update'
    end
    assert page.has_link?('Canicule')
  end


  test 'Update an entity with wrong locale' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_link?('soleil')
    within('#entities-list') do
      click_link 'soleil'
      assert page.has_text?('Cancel')
      fill_in 'Terms', with: "Canicule:xx"
      click_button 'Update'
    end
    assert page.has_text?("Terms unknown locale 'xx'")
  end


  test 'Show an entity with details' do
    login_as 'show_on_agent_weather@viky.ai', 'BimBamBoom'
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('Entities lists')
    click_link 'Entities lists'

    assert page.has_text?('weather_conditions')
    click_link 'weather_conditions'
    assert page.has_text?('Entities lists / weather_conditions PUBLIC')
    within('#entities-list') do
      click_link 'soleil'
      assert page.has_no_button?('Update')
    end
  end


  test 'Delete an entity' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_link?('pluie')
    within('#entities-list') do
      click_link 'pluie'
      assert page.has_text?('Delete')
      all('a').last.click
    end
    assert page.has_no_link?('Delete')
  end


  test 'Delete all entities' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_link?('pluie')
    within('#entities-list') do
      click_link 'pluie'
      assert page.has_text?('Delete')
      all('a').last.click
      click_link 'soleil'
      assert page.has_text?('Delete')
      all('a').last.click
    end
    assert page.has_text?('Start adding entity')
  end


  test 'Entity autosolution synchronization' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    within('.entity-form') do
      assert page.has_no_text?('brumeux')
      check('Auto solution')
      page.execute_script %Q{ $('#terms__new_entity').trigger("click") }
      fill_in 'Terms', with: "  \nbrumeux"
      assert page.has_text?('"brumeux"')

      uncheck('Auto solution')
      fill_in 'Terms', with: "foggy\nbrumeux"
      assert page.has_text?('"brumeux"')
      check('Auto solution')
      assert page.has_text?('"foggy"')
    end
  end


  private

    def admin_go_to_entities_list_show(agent, entities_list)
      admin_login
      visit user_agent_entities_list_path(users(:admin), agent, entities_list)
      assert page.has_text?("#{entities_list.listname} PUBLIC")
    end

    def fill_in_editor_field(text)
      within '.CodeMirror' do
        # Click makes CodeMirror element active:
        current_scope.click
        # Find the hidden textarea:
        field = current_scope.find('textarea', visible: false)
        # Mimic user typing the text:
        field.send_keys text
      end
    end
end
