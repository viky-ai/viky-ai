require "application_system_test_case"

class AgentsDependenciesTest < ApplicationSystemTestCase


  test "Add agent dependency" do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert page.has_text?('Agents / My awesome weather bot (admin/weather)')
    assert page.has_text?(' Agent dependencies (0) - Dependents (0) ')

    click_link "Add new dependency"

    within(".modal") do
      assert_equal ["T-800 admin/terminator"], all('a').collect(&:text)
      click_link "T-800 admin/terminator"
    end

    assert page.has_text?('T-800 admin/terminator')
    assert page.has_no_text?('Add new dependency')
    assert page.has_text?(' Agent dependencies (1) - Dependents (0) ')
  end


  test "Use alias from sucessor agent" do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert page.has_text?('Agents / My awesome weather bot (admin/weather)')
    assert page.has_text?(' Agent dependencies (0) - Dependents (0) ')

    # Add dependency
    click_link "Add new dependency"
    within(".modal") do
      assert_equal ["T-800 admin/terminator"], all('a').collect(&:text)
      click_link "T-800 admin/terminator"
    end
    assert page.has_text?('T-800 admin/terminator')

    # Edit an interpretation
    click_link 'weather_greeting'
    assert page.has_text?('weather_greeting (admin/weather/weather_greeting)')

    # Add Expression & create an alias from dependency
    first('trix-editor').click.set('Salut Marcel')
    select_text_in_trix("trix-editor", 6, 12)
    find_link('admin/terminator/terminator_find').click
    click_button 'Add'

    assert page.has_text?("Salut Marcel")
    assert_equal 2, all('.interpretation-resume').count
    assert_equal 1, all('span[title="admin/terminator/terminator_find"]').size
  end


end
