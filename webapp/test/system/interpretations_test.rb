require 'application_system_test_case'

class InterpretationsTest < ApplicationSystemTestCase

  test 'Create an interpretation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')
    fill_in 'expression', with: 'Good morning'
    click_button 'Add'
    assert page.has_text?('Good morning')
  end

  test 'Errors on interpretation creation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')
    fill_in 'expression', with: ''
    click_button 'Add'
    assert page.has_text?('Expression can\'t be blank')
  end

  test 'Delete an interpretation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Hello world')
    click_button 'Delete'
    assert page.has_no_text?('Hello world')
  end
end
