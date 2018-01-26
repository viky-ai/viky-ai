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
end
