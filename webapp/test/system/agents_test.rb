require "application_system_test_case"

class AgentsTest < ApplicationSystemTestCase

  test 'Navigation to agents index' do
    go_to_agents_index
    assert page.has_content?("Agents management")
    assert_equal "My awesome weather bot", first('.agents-list__item h3').text
  end
end
