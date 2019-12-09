require "application_system_test_case"

class BackendDashboardTest < ApplicationSystemTestCase

  test "User index not allowed if user is not logged in" do
    visit backend_dashboard_path
    assert has_text?("Please, log in before continuing.")
    assert_equal "/users/sign_in", current_path
  end


  test "User index not allowed if user is not admin" do
    login_as "confirmed@viky.ai", "BimBamBoom"
    visit backend_dashboard_path
    assert has_text?("You do not have permission to access this interface.")
    assert_equal "/agents", current_path
  end


  test "Successful access" do
    admin_login

    visit backend_dashboard_path
    assert_equal "/backend/dashboard", current_path
    assert has_text?("4\nAgents")
    assert has_text?("One public agent")
    assert has_text?("6\nFormulations")
    assert has_text?("5 interpretations")
    assert has_text?("8\nEntities")
    assert has_text?("5 entities lists")
    assert has_text?("0\nTest")
    assert has_text?("1\nReadme")
    assert has_text?("25% of agents")
    assert has_text?("2\nBots")
    assert has_text?("One WIP")
    assert has_text?("7\nUsers")
    assert has_text?("One administrator")

    assert has_text?("Agents with most formulations")
    assert has_text?("Agents with most entities")
    assert has_text?("Agents most used as dependency")
    assert has_text?("Agents with most tests in failure")
  end

end
