require "test_helper"
require 'capybara/poltergeist'

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  driven_by :poltergeist
  #driven_by :selenium, using: :chrome, screen_size: [1400, 1400]

  def admin_login
    visit new_user_session_path
    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'
    click_button 'Log in'
  end

  def go_to_agents_index
    admin_login
    within(".nav") do
      click_link "Agents"
    end
  end
end
