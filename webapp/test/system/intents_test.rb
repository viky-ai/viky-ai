require 'application_system_test_case'

class IntentsTest < ApplicationSystemTestCase

  test 'Create an intent' do
    go_to_agent_intents('admin', 'terminator')
    click_link 'New interpretation'
    within('.modal') do
      assert page.has_text? 'Create a new interpretation'
      fill_in 'ID', with: 'sunny_day'
      fill_in 'Description', with: 'Questions about the next sunny day'
      click_button 'Private'
      click_button 'Create'
    end
    assert page.has_text?('Interpretation has been successfully created.')
  end

  test 'Errors on intent creation' do
    go_to_agent_intents('admin', 'terminator')
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
    go_to_agent_intents('admin', 'weather')
    within '#intents-list-is_public' do
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
    go_to_agent_intents('admin', 'weather')
    within '#intents-list-is_public' do
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
    assert page.has_text?('Interpretation with the name: weather_forecast has successfully been deleted.')

    agent = agents(:weather)
    assert_equal user_agent_intents_path(agent.owner, agent), current_path
  end


  test 'Reorganize intents' do
    intent = Intent.new(intentname: 'test', locales: ['en'])
    intent.agent = agents(:weather)
    assert intent.save

    go_to_agent_intents('admin', 'weather')
    assert_equal ['test', 'weather_forecast', 'weather_question'], all('.card-list__item__name').collect(&:text)

    assert_equal 3, all('.card-list__item__draggable').size

    # Does not works...
    # first_item = all('.intents-list__item__draggable').first
    # last_item  = all('.intents-list__item__draggable').last
    # first_item.drag_to(last_item)

    # assert_equal ['weather_forecast', 'test'], all('.intents-list__item__name').collect(&:text)
  end


  test 'Add locale to an intent' do
    go_to_agent_intents('admin', 'terminator')
    click_link 'terminator_find'

    assert page.has_text?('+')
    assert page.has_no_text?('fr')
    click_link '+'
    within('.modal') do
      assert page.has_text?('Choose a language')
      click_link('fr (French)')
    end
    assert page.has_text?('fr')
  end


  test 'Remove locale of an intent' do
    go_to_agent_intents('admin', 'weather')
    click_link 'weather_forecast'

    assert page.has_link?('en')
    assert page.has_link?('fr')

    within('.card') do
      click_link 'en'
    end
    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      all('a').last.click
    end

    assert page.has_text?('Do you want to remove it ?')
    click_link 'Yes, remove "en" tab'
    within('.card') do
      assert page.has_no_link?('en')
      assert page.has_link?('fr')
    end

    assert page.has_text?('Quel temps fera-t-il demain ?')

    within('#interpretations-list') do
      click_link 'Quel temps fera-t-il demain ?'
      assert page.has_text?('Cancel')
      all('a').last.click
    end
    assert page.has_text?('Start adding expressions using the form below.')
  end


  test 'Move intent to another agent' do
    intent = intents(:weather_question)
    intent.visibility = Intent.visibilities[:is_private]
    assert intent.save

    go_to_agent_intents('admin', 'weather')
    within '#intents-list-is_private' do
      first('.dropdown__trigger > button').click
      assert page.has_no_link?('Move to T-800')
      click_link 'Move to...'
    end

    assert page.has_text?('Move weather_question to ')
    within('.modal') do
      click_link 'T-800'
    end

    assert page.has_text?('Intent weather_question moved to agent T-800')
    assert page.has_link?('T-800')

    within '#intents-list-is_public' do
      first('.dropdown__trigger > button').click
      assert page.has_link?('Move to T-800')
    end

    go_to_agent_intents('admin', 'terminator')
    within '#intents-list-is_private' do
      assert page.has_text?('weather_question')
      first('.dropdown__trigger > button').click
      assert page.has_no_link?('Move to T-800')
    end
  end


  test 'Filter favorite agent select' do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: 'edit')
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    go_to_agent_intents('admin', 'terminator')
    within '#intents-list-is_public' do
      first('.dropdown__trigger > button').click
      click_link 'Move to'
    end

    assert page.has_text?('Move terminator_find to ')
    within('.modal') do
      click_button 'Favorites'
      assert page.has_no_text?('My awesome weather bot admin/weather')
      assert page.has_text?('Weather bot confirmed/weather')
    end
  end


  test 'Filter query agent dependency' do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: 'edit')
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    go_to_agent_intents('admin', 'terminator')
    within '#intents-list-is_public' do
      first('.dropdown__trigger > button').click
      click_link 'Move to'
    end

    assert page.has_text?('Move terminator_find to ')
    within(".modal") do
      fill_in 'search_query', with: 'awesome'
      click_button '#search'
      assert page.has_text?('My awesome weather bot admin/weather')
      assert page.has_no_text?('Weather bot confirmed/weather')
    end
  end

  test 'Used by menu' do
    go_to_agent_intents('admin', 'terminator')

    within '#intents-list-is_public' do
      assert first('li').has_no_link?('Used by...')
    end

    page.refresh

    within '#intents-list-is_private' do
      assert first('li').has_link?('Used by...')
      # click link and check
      click_link 'Used by...'
    end

    assert page.has_text?('Interpretations using simple_where')
    within '.modal' do
      within '#intents-list-is_public' do
        assert page.has_link?('terminator_find')
      end

      within '#intents-list-is_private' do
        assert_no_css('li')
      end
    end
  end
end
