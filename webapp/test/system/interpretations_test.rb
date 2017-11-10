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

  test 'Update an intent' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_link?('Hello world')
    within('#interpretations-list') do
      click_link 'Hello world'
      fill_in 'interpretation[expression]', with: 'Hello every body'
      click_button 'Update'
    end
    assert page.has_link?('Hello every body')
  end

  test 'Delete an interpretation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_link?('Hello world')
    within('#interpretations-list') do
      click_link 'Hello world'
      click_button 'Delete'
    end
    assert page.has_no_link?('Hello world')
  end
end
