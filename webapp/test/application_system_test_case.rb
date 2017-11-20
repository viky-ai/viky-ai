require "test_helper"

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  driven_by :headless_chrome
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
    all('.h-nav a').last.click
  end

  def go_to_agents_index
    admin_login
    assert page.has_text?("Agents")
  end

  def go_to_agent_show(user, agent)
    admin_login
    visit user_agent_path(user, agent)
    assert page.has_text?("Agent intents")
  end

end
