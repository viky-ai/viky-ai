require 'application_system_test_case'

class IntentsNewTest < ApplicationSystemTestCase

  test 'Create an intent' do
    go_to_agents_index
    assert page.has_text?('admin/terminator')
    click_link 'T-800'
    click_link 'New intent'

    within('.modal') do
      assert page.has_text? 'Create intent'
      fill_in 'ID', with: 'sunny_day'
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Create'
    end
    assert page.has_text?('Intent has been successfully created.')
  end

  test 'Errors on intent creation' do
    go_to_agents_index
    assert page.has_text?('admin/terminator')
    click_link 'T-800'
    click_link 'New intent'

    within('.modal') do
      assert page.has_text? 'Create intent'
      fill_in 'ID', with: ''
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Create'
      assert page.has_text?('ID is too short (minimum is 3 characters)')
      assert page.has_text?('ID can\'t be blank')
    end
  end

end
