require "test_helper"
require 'capybara/poltergeist'

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  driven_by :poltergeist
  #driven_by :selenium, using: :chrome, screen_size: [1400, 1400]

  def login_as(login, password)
    visit new_user_session_path
    fill_in 'Email', with: login
    fill_in 'Password', with: password
    click_button 'Log in'
  end

  def admin_login
    login_as 'admin@viky.ai', 'AdminBoom'
  end

  def logout
    first('.nav__footer svg').click
  end

  def go_to_agents_index
    admin_login
    assert page.has_text?("Agents")
  end
end
