require "application_system_test_case"

class AgentsShowTest < ApplicationSystemTestCase

  test 'Navigation to agent show' do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert page.has_text?('admin/weather')
    assert page.has_text?('Sharing overview')
    assert_equal '/agents/admin/weather', current_path
  end


  test 'Navigation to agent show without rights' do
    admin_login
    visit user_agent_path(users(:confirmed), agents(:weather_confirmed))
    assert page.has_text?("Unauthorized operation.")
  end

  test "Navigation to agent show when user is a collaborator in another agent with same name" do
    user = users(:show_on_agent_weather)

    new_agent = Agent.new(
      name: "Weather agent 2",
      agentname: agents(:weather).agentname,
      description: "Agent A decription",
      visibility: 'is_public',
      nlp_updated_at: '2019-01-21 10:07:53.484942',
    )
    new_agent.memberships << Membership.new(user_id: user.id, rights: "all")
    assert new_agent.save

    login_as(user.email, 'BimBamBoom')
    go_to_agents_index
    fill_in 'search_query', with: 'weather'
    click_button '#search'

    assert page.has_text?('My awesome weather bot admin/weather')
    assert page.has_text?('Weather agent 2 show_on_agent_weather/weather')
    
    click_link 'Weather agent 2 show_on_agent_weather/weather'
    assert_equal '/agents/show_on_agent_weather/weather', current_path
    assert page.has_text?('Weather agent 2 show_on_agent_weather/weather')
  end


  test 'No redirection when configuring from show' do
    go_to_agents_index
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('Sharing overview')
    click_link 'Configure'
    assert page.has_text?('Configure agent')
    click_button 'Cancel'
    assert_equal '/agents/admin/weather', current_path

    click_link 'Configure'
    assert page.has_text?('Configure agent')
    fill_in 'Name', with: 'My new updated weather agent'
    fill_in 'ID', with: 'weather-v2'
    click_button 'Update'
    assert page.has_text?('admin/weather-v2')
    assert_equal '/agents/admin/weather-v2', current_path
  end


  test 'Transfer agent ownership' do
    go_to_agents_index
    assert page.has_content?('admin/terminator')
    click_link 'T-800'
    click_link 'Transfer ownership'
    within(".modal") do
      page.execute_script "document.getElementById('input-user-search').value = '#{users('confirmed').id}'"
      click_button 'Transfer'
    end
    assert page.has_text?('Agent T-800 transferred to user confirmed')
    assert_equal '/agents', current_path
    assert page.has_no_content?('admin/terminator')
  end


  test 'Transfer agent ownership whereas another agent with the same agentname throw an error' do
    go_to_agents_index
    assert page.has_content?('admin/weather')
    click_link 'My awesome weather bot'
    click_link 'Transfer ownership'
    within(".modal") do
      page.execute_script "document.getElementById('input-user-search').value = '#{users('confirmed').id}'"
      click_button 'Transfer'
      assert page.has_content?('This user already have an agent with this ID')
    end
  end


  test 'Share from agent show if owner' do
    go_to_agents_index
    click_link 'T-800'
    click_link 'Share'
    click_link 'Invite collaborators'
    assert page.has_content?('Share with')
    within(".modal") do
      page.execute_script "document.getElementById('input-user-search').value = '#{users('confirmed').id}'"
      click_button 'Invite'
    end
    assert page.has_text?('Agent terminator shared with : confirmed.')
    assert_equal '/agents/admin/terminator', current_path
  end


  test 'Cannot share from agent show if not owner' do
    login_as users(:edit_on_agent_weather).email, 'BimBamBoom'

    assert page.has_text?("Agents")

    click_link 'My awesome weather bot'
    assert_equal '/agents/admin/weather', current_path
    assert page.has_no_link?('Share')
  end


  test 'Access to unkown agent' do
    go_to_agents_index
    visit user_agent_path('admin', 'unknown-agent')
    assert_equal '/404', current_path
  end


  test 'Add an owned agent to favorites' do
    go_to_agents_index
    click_link 'My awesome weather bot admin/weather'
    click_link 'Add favorite'
    assert page.has_text?('Remove favorite')
  end


  test 'Add a public agent to favorites' do
    login_as 'confirmed@viky.ai', 'BimBamBoom'
    assert page.has_text?("Agents")
    click_link 'T-800 admin/terminator'
    click_link 'Add favorite'
    assert page.has_text?('Remove favorite')
  end


  test 'Remove an agent to favorites' do
    admin = users(:admin)
    weather = agents(:weather)
    assert FavoriteAgent.create(user: admin, agent: weather)

    go_to_agents_index
    click_link 'My awesome weather bot admin/weather'
    click_link 'Remove favorite'
    assert page.has_text?('Add favorite')
  end


  test 'Duplicate an agent' do
    assert Readme.create(
      agent: agents(:weather),
      content: 'My awesome readme !!!'
    )

    go_to_agent_show(users(:admin), agents(:weather))

    perform_enqueued_jobs do
      DuplicateAgentJob.perform_later(agents(:weather), users(:admin)) # click_link 'Duplicate'
    end

    go_to_agents_index
    assert page.has_text?('admin/weather-copy')

    click_link 'admin/weather-copy'
    assert page.has_text?('My awesome readme !!!')
    assert page.has_link?('Interpretations 2')
    assert page.has_link?('Entities 2')

    go_to_agent_intents('admin', 'weather-copy')
    click_link 'weather_forecast'
    assert page.has_text?('Interpretations / weather_forecast PUBLIC')

    within('.card .tabs') do
      click_link 'en'
    end

    assert page.has_link?('What the weather like tomorrow ?')

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('admin/weather-copy/interpretations/weather_question')
      assert page.has_link?('admin/weather-copy/interpretations/weather_question')
    end
  end
end
