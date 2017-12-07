require "application_system_test_case"

class AgentsDependenciesTest < ApplicationSystemTestCase

  test "Navigation to agent creation" do
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

end
