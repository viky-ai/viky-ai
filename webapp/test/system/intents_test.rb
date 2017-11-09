require 'application_system_test_case'

class IntentsTest < ApplicationSystemTestCase

  test 'Create an intent' do
    go_to_agents_index
    assert page.has_text?('admin/terminator')
    click_link 'T-800'
    click_link 'New intent'

    within('.modal') do
      assert page.has_text? 'Create a new intent'
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
      assert page.has_text? 'Create a new intent'
      fill_in 'ID', with: ''
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Create'
      assert page.has_text?('ID is too short (minimum is 3 characters)')
      assert page.has_text?('ID can\'t be blank')
    end
  end

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

  test 'Delete an intent' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    within '.intents-list' do
      first('.dropdown__trigger > button').trigger('click')
      click_link 'Delete'
    end

    within('.modal') do
      assert page.has_text?('Are you sure?')
      click_button('Delete')
      assert page.has_text?('Please enter the text exactly as it is displayed to confirm.')

      fill_in 'validation', with: 'DELETE'
      click_button('Delete')
    end
    assert page.has_text?('Intent with the name: weather_greeting has successfully been deleted.')

    agent = agents(:weather)
    assert_equal user_agent_path(agent.owner, agent), current_path
  end


  test 'reorganize intents' do
    go_to_agent_show('admin', 'weather')
    assert page.has_no_text? 'Reorganize intents'

    intent = Intent.new(intentname: 'test')
    intent.agent = agents(:weather)
    assert intent.save
    visit user_agent_path('admin', 'weather')
    assert_equal ['test', 'weather_greeting'], all('.intents-list__item__name').collect(&:text)
    assert_equal 0, all('.btn--drag').size

    click_link('Reorganize intents')
    assert page.has_text? 'Done, i finished reorganizing intents'

    assert_equal 2, all('.btn--drag').size

    # Does not work...
    # first('.btn--drag').native.drag_by(0, 100)

    click_link('Done, i finished reorganizing intents')
    assert_equal 0, all('.btn--drag').size
  end

end
