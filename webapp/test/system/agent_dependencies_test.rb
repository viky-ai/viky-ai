require "application_system_test_case"
require 'model_test_helper'

class AgentsDependenciesTest < ApplicationSystemTestCase


  test "Add agent dependency" do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Dependencies (0) - Dependents (0)")

    # Add admin/terminator to admin/weather
    click_link "Add new dependency"
    within(".modal") do
      assert_equal ["PUBLIC\nT-800\nadmin/terminator"], all('a').collect(&:text)
      click_link "T-800 admin/terminator"
    end

    assert has_text?("T-800\nadmin/terminator")
    assert has_no_text?("Add new dependency")
    assert has_text?("Dependencies (1) - Dependents (0)")

    # Try to add admin/weather to admin/terminator and detect cycle
    click_link "T-800 admin/terminator"
    click_link "Add new dependency"

    within(".modal") do
      assert_equal ["My awesome weather bot\nadmin/weather"], all('a').collect(&:text)
      click_link "My awesome weather bot admin/weather"
    end

    assert has_text?('agent addition would cause a cycle in dependency graph')
  end


  test "Use alias from successor agent and delete dependency" do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Dependencies (0) - Dependents (0)")

    # Add dependency
    click_link "Add new dependency"
    within(".modal") do
      assert_equal ["PUBLIC\nT-800\nadmin/terminator"], all('a').collect(&:text)
      click_link "T-800 admin/terminator"
    end

    # Delete ? no
    click_link "Delete"
    within(".modal") do
      assert has_text?('No reference to agent "T-800" interpretations')
      click_link "Cancel"
    end

    # Edit an interpretation
    assert has_link?("Interpretations")
    click_link "Interpretations"
    click_link "weather_forecast"

    within(".header__breadcrumb") do
      assert has_text?("Interpretations / weather_forecast PUBLIC")
    end

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")

    # Add Expression & create an alias from dependency
    first("trix-editor").click.set("Salut Marcel")
    select_text_in_trix("trix-editor", 6, 12)
    within "#popup-add-tag" do
      find_link("admin/terminator/interpretations/terminator_find").click
    end
    click_button "Add"

    assert has_text?("Salut Marcel")
    assert_equal 2, all(".interpretation-resume").count
    assert_equal 1, all('span[title="admin/terminator/interpretations/terminator_find"]').size

    # Return to interpretation list
    click_link "Overview"

    # Delete dependency
    click_link "Delete"
    within(".modal") do
      assert has_text?('One reference to agent "T-800" interpretations')
      fill_in "validation", with: "DELETE"
      click_button("Delete")
    end
    assert has_no_text?("T-800 admin/terminator")

    # Edit an interpretation
    assert has_link?("Interpretations")
    click_link "Interpretations"
    click_link "weather_forecast"

    within(".card .tabs") do
      click_link "en"
    end

    assert has_text?("Salut Marcel")
    assert_equal 1, all(".interpretation-resume").count
    assert_equal 0, all('span[title="admin/terminator/interpretations/terminator_find"]').size
  end


  test "Cannot add dependency if no edit right" do
    create_agent("dependency_test", :show_on_agent_weather)

    login_as "show_on_agent_weather@viky.ai", "BimBamBoom"
    assert has_text?("Agents")
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Dependencies (0) - Dependents (0)")
    assert has_no_link?("Add new dependency")
  end


  test "Filter favorite agent dependency" do
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = Agent.visibilities[:is_public]
    assert agent_public.save
    admin = users(:admin)
    assert FavoriteAgent.create(user: admin, agent: agent_public)

    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Dependencies (0) - Dependents (0)")

    click_link "Add new dependency"
    within(".modal") do
      click_button "Favorites"
      assert has_text?("PUBLIC\nWeather bot\nconfirmed/weather")
      assert has_no_text?("PUBLIC\nT-800\nadmin/terminator")
    end
  end


  test "Filter query agent dependency" do
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = Agent.visibilities[:is_public]
    assert agent_public.save

    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"
    assert has_text?("Dependencies (0) - Dependents (0)")

    click_link "Add new dependency"
    within(".modal") do
      fill_in "search_query", with: "T-800"
      click_button "#search"
      assert has_no_text?("PUBLIC\nWeather bot\nconfirmed/weather")
      assert has_text?("PUBLIC\nT-800\nadmin/terminator")
    end
  end


  test 'Delete the expected dependency even when there is a duplicate agentname on another user' do
    terminator = agents(:terminator)
    weather_confirmed = agents(:weather_confirmed)
    assert AgentArc.create(source: terminator, target: weather_confirmed)

    admin_go_to_agents_index
    click_link "T-800 admin/terminator"
    within(".agents-compact-grid") do
      assert has_text?("Weather bot\nconfirmed/weather")
      click_link "Delete"
    end
    within(".modal") do
      assert has_text?("Type DELETE to confirm")
      fill_in "validation", with: "DELETE"
      click_button("Delete")
    end
    within(".agents-compact-grid") do
      assert has_no_text?("Weather bot\nconfirmed/weather")
    end
  end
end
