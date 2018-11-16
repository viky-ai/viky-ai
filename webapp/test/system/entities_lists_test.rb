require 'application_system_test_case'

class EntitiesListsTest < ApplicationSystemTestCase

  test 'Create an entities list' do
    go_to_agent_entities_lists('admin', 'terminator')
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
    go_to_agent_entities_lists('admin', 'terminator')
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


  test 'Update an entities list' do
    go_to_agent_entities_lists('admin', 'weather')
    within '#entities_lists-list-is_public' do
      first('.dropdown__trigger > button').click
      click_link 'Configure'
    end

    within('.modal') do
      assert page.has_text? 'Edit entities list'
      fill_in 'ID', with: 'countries'
      fill_in 'Description', with: 'Every countries'
      click_button 'Update'
    end
    assert page.has_text?('Your entities list has been successfully updated.')
  end


  test 'Delete an entities list' do
    go_to_agent_entities_lists('admin', 'weather')
    within '#entities_lists-list-is_public' do
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
    assert page.has_text?('Entities list with the name: weather_conditions has successfully been deleted.')

    agent = agents(:weather)
    assert_equal user_agent_entities_lists_path(agent.owner, agent), current_path
  end


  test 'Reorganize entities lists' do
    entities_list = EntitiesList.new(listname: 'test', agent: agents(:weather))
    assert entities_list.save
    go_to_agent_entities_lists('admin', 'weather')
    assert page.has_link? 'New entities list'
    # assert_equal ['test', 'weather_conditions'], all('.card-list__item__name').collect(&:text)
    #
    # assert_equal 2, all('.card-list__item__draggable').size
    #
    # Does not works...
    # first_item = all('.card-list__item__draggable).first
    # last_item  = all('.card-list__item__draggable).last
    # first_item.drag_to(last_item)

    # assert_equal ['weather_conditions', 'test'], all('.card-list__item__name').collect(&:text)
  end


  test 'Move entities list to another agent' do
    assert EntitiesList.create(listname: 'Other list', agent: agents(:terminator))

    go_to_agent_entities_lists('admin', 'terminator')
    within '#entities_lists-list-is_private' do
      first('.dropdown__trigger > button').click
      assert page.has_no_link?('Move to My awesome weather bot')
      click_link 'Move to'
    end

    assert page.has_text?('Move terminator_targets to ')
    within('.modal') do
      click_link 'My awesome weather bot'
    end

    assert page.has_text?('Entities list terminator_targets moved to agent My awesome weather bot')
    assert page.has_link?('My awesome weather bot')

    within '#entities_lists-list-is_public' do
      first('.dropdown__trigger > button').click
      assert page.has_link?('Move to My awesome weather bot')
    end

    go_to_agent_entities_lists('admin', 'weather')
    within '#entities_lists-list-is_private' do
      assert page.has_text?('terminator_targets')
      first('.dropdown__trigger > button').click
      assert page.has_no_link?('Move to My awesome weather bot')
    end
  end


  test 'Filter favorite agent select' do
    admin = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.memberships << Membership.new(user: admin, rights: 'edit')
    assert agent_public.save
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    go_to_agent_entities_lists('admin', 'terminator')
    within '#entities_lists-list-is_private' do
      first('.dropdown__trigger > button').click
      click_link 'Move to'
    end

    assert page.has_text?('Move terminator_targets to ')
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

    go_to_agent_entities_lists('admin', 'terminator')
    within '#entities_lists-list-is_private' do
      first('.dropdown__trigger > button').click
      click_link 'Move to'
    end

    assert page.has_text?('Move terminator_targets to ')
    within(".modal") do
      fill_in 'search_query', with: 'awesome'
      click_button '#search'
      assert page.has_text?('My awesome weather bot admin/weather')
      assert page.has_no_text?('Weather bot confirmed/weather')
    end
  end

  test 'aliased intents' do
    go_to_agent_entities_lists('admin', 'weather')

    within '#entities_lists-list-is_public' do
      assert all('li').first.has_no_link?('Used by...')
      assert all('li').last.has_link?('Used by...')
      click_link('Used by...')
    end

    within '.modal' do
      assert page.has_text?('Interpretations using weather_dates')
      assert page.has_link?('weather_forecast')
      click_link('weather_forecast')
    end

    within('.aliased-intents-list') do
      assert page.has_link?('weather_forecast')
    end
    expected = "/agents/admin/weather/interpretations#intent-#{intents(:weather_forecast).id}"
    assert current_url.include?(expected)
  end

end
