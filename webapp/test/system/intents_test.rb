require 'application_system_test_case'

class IntentsTest < ApplicationSystemTestCase

  test 'Create an intent' do
    go_to_agent_show('admin', 'terminator')
    click_link 'New interpretation'
    within('.modal') do
      assert page.has_text? 'Create a new interpretation'
      fill_in 'ID', with: 'sunny_day'
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Create'
    end
    assert page.has_text?('Interpretation has been successfully created.')
  end


  test 'Errors on intent creation' do
    go_to_agent_show('admin', 'terminator')
    click_link 'New interpretation'
    within('.modal') do
      assert page.has_text? 'Create a new interpretation'
      fill_in 'ID', with: ''
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Create'
      assert page.has_text?('ID is too short (minimum is 3 characters)')
      assert page.has_text?('ID can\'t be blank')
    end
  end


  test 'Update an intent' do
    go_to_agent_show('admin', 'weather')
    within '.intents-list' do
      first('.dropdown__trigger > button').click
      click_link 'Configure'
    end

    within('.modal') do
      assert page.has_text? 'Edit interpretation'
      fill_in 'ID', with: 'sunny_day'
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Update'
    end
    assert page.has_text?('Your interpretation has been successfully updated.')
  end


  test 'Delete an intent' do
    go_to_agent_show('admin', 'weather')
    within '.intents-list' do
      first('.dropdown__trigger > button').click
      click_link 'Delete'
    end

    within('.modal') do
      assert page.has_text?('Are you sure?')
      click_button('Delete')
      assert page.has_text?('Please enter the text exactly as it is displayed to confirm.')

      fill_in 'validation', with: 'DELETE'
      click_button('Delete')
    end
    assert page.has_text?('Interpretation with the name: weather_greeting has successfully been deleted.')

    agent = agents(:weather)
    assert_equal user_agent_path(agent.owner, agent), current_path
  end


  test 'Reorganize intents' do
    go_to_agent_show('admin', 'weather')

    intent = Intent.new(intentname: 'test', locales: ['en-US'])
    intent.agent = agents(:weather)
    assert intent.save
    visit user_agent_path('admin', 'weather')
    assert_equal ['test', 'weather_greeting'], all('.intents-list__item__name').collect(&:text)

    assert_equal 2, all('.intents-list__item__draggable').size

    # Does not work...
    # first('.intents-list__draggable').native.drag_by(0, 100)

    # assert_equal ['weather_greeting', 'test'], all('.intents-list__item__name').collect(&:text)
  end


  test 'Add locale to an intent' do
    go_to_agent_show('admin', 'terminator')
    click_link 'terminator_find'

    assert page.has_text?('+')
    assert page.has_no_text?('fr-FR')
    click_link '+'
    within('.modal') do
      assert page.has_text?('Choose a language')
      click_link('fr-FR')
    end
    assert page.has_text?('fr-FR')
  end


  test 'Remove locale of an intent' do
    go_to_agent_show('admin', 'weather')
    click_link 'weather_greeting'

    assert page.has_link?('en-US')
    assert page.has_link?('fr-FR')

    click_link 'en-US'
    within('#interpretations-list') do
      click_link 'Hello world'
      assert page.has_text?('Cancel')
      all('a').last.click
    end

    assert page.has_text?('Do you want to remove it ?')
    click_link 'Yes, remove "en-US" tab'
    assert page.has_no_link?('en-US')
    assert page.has_link?('fr-FR')

    assert page.has_text?('Bonjour tout le monde')

    within('#interpretations-list') do
      click_link 'Bonjour tout le monde'
      assert page.has_text?('Cancel')
      all('a').last.click
    end
    assert page.has_text?('Start adding expressions using the form below.')
  end
end
