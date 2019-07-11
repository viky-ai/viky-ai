require "application_system_test_case"

class PlayTest < ApplicationSystemTestCase

  test "Display blank slate" do
    admin_login
    within(".h-nav") do
      click_link "Play"
    end
    assert page.has_text?("Welcome to playground!")
  end

end
