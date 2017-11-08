require 'application_system_test_case'

class IntentsTest < ApplicationSystemTestCase
  test 'Update an intent' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'

    assert page.has_text?('weather_greeting')

    within '.intents-list' do
      first('.dropdown__trigger > button').trigger('click')
      click_link 'Configure'
    end

    within('.modal') do
      assert page.has_text? 'Edit intent'
      fill_in 'ID', with: 'sunny_day'
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Update'
    end
    assert page.has_text?('Your intent has been successfully updated.')
  end
end
