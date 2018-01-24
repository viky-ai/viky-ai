require 'application_system_test_case'

class EntitiesListsTest < ApplicationSystemTestCase

  test 'Create an entities list' do
    go_to_agent_show('admin', 'terminator')
    click_link 'New entities list'
    within('.modal') do
      assert page.has_text? 'Create a new entities list'
      fill_in 'ID', with: 'towns'
      fill_in 'Description', with: 'List every towns in the world !'
      click_button 'Private'
      click_button 'Create'
    end
    assert page.has_text?('Entities list has been successfully created.')
  end


  test 'Errors on entities list creation' do
    go_to_agent_show('admin', 'terminator')
    click_link 'New entities list'
    within('.modal') do
      assert page.has_text? 'Create a new entities list'
      fill_in 'ID', with: ''
      fill_in 'Description', with: 'List every towns in the world !'
      click_button 'Create'
      assert page.has_text?('ID is too short (minimum is 3 characters)')
      assert page.has_text?('ID can\'t be blank')
    end
  end
end
