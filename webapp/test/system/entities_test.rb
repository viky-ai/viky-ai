require 'application_system_test_case'

class InterpretationsTest < ApplicationSystemTestCase

  test 'Navigate to an entity' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('Entities lists')
    click_link 'Entities lists'

    assert page.has_text?('weather_conditions')
    click_link 'weather_conditions'
    assert page.has_text?('weather_conditions PUBLIC (admin/weather/weather_conditions)')
  end


  test 'Create an entity' do
    admin_go_to_entities_list_show(agents(:weather), entities_lists(:weather_conditions))
    assert page.has_no_text?('foggy')
    within('.entity-form') do
      fill_in 'terms__new_entity', with: "foggy\nbrumeux"
      uncheck('Auto solution')
      fill_in 'solution__new_entity', with: 'condition: foggy'
      click_button 'Add'
    end
    assert page.has_text?('foggy')
  end


  test 'Show an entity with details' do
    login_as 'show_on_agent_weather@viky.ai', 'BimBamBoom'
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('Entities lists')
    click_link 'Entities lists'

    assert page.has_text?('weather_conditions')
    click_link 'weather_conditions'
    assert page.has_text?('weather_conditions PUBLIC (admin/weather/weather_conditions)')
    within('#entities-list') do
      click_link 'soleil'
      assert page.has_no_button?('Update')
    end
  end


  private

    def admin_go_to_entities_list_show(agent, entities_list)
      admin_login
      visit user_agent_entities_list_path(users(:admin), agent, entities_list)
      assert page.has_text?("#{entities_list.listname} PUBLIC (#{users(:admin).username}/#{agent.agentname}/#{entities_list.listname})")
    end
end
