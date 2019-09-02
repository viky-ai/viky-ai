require "application_system_test_case"

class BackendDashboardTest < ApplicationSystemTestCase

  test "User index not allowed if user is not logged in" do
    visit backend_dashboard_path
    assert has_content?("Please, log in before continuing.")
    assert_equal "/users/sign_in", current_path
  end


  test "User index not allowed if user is not admin" do
    login_as "confirmed@viky.ai", "BimBamBoom"
    visit backend_dashboard_path
    assert has_content?("You do not have permission to access this interface.")
    assert_equal "/agents", current_path
  end


  test "Successful access" do
    admin_login

    visit backend_dashboard_path
    assert_equal "/backend/dashboard", current_path
    assert has_content?("4\nAgents")
    assert has_content?("One public agent")
    assert has_content?("6\nInterpretations")
    assert has_content?("5 intents")
    assert has_content?("8\nEntities")
    assert has_content?("5 entities lists")
    assert has_content?("0\nTest")
    assert has_content?("1\nReadme")
    assert has_content?("25% of agents")
    assert has_content?("2\nBots")
    assert has_content?("One WIP")
    assert has_content?("7\nUsers")
    assert has_content?("One administrator")

    assert has_content?("Agents with most interpretations")
    assert has_content?("Agents with most entities")
    assert has_content?("Agents most used as dependency")
    assert has_content?("Agents with most tests in failure")
  end

end
