require "test_helper"

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase

  Capybara.register_driver(:headless_chrome) do |app|
    capabilities = Selenium::WebDriver::Remote::Capabilities.chrome(
      chromeOptions: {
        args: %w{headless no-sandbox window-size=1200,720}
      }
    )
    driver_options = {
      browser: :chrome,
      desired_capabilities: capabilities
    }
    if File.exist?('/etc/os-release') && `cat /etc/os-release` =~ /ubuntu/i
      driver_options[:driver_path] = '/usr/lib/chromium-browser/chromedriver'
    end
    Capybara::Selenium::Driver.new(app, driver_options)
  end

  driven_by :headless_chrome
  #driven_by :selenium, using: :chrome, screen_size: [1400, 1400]

  def login_as(login, password)
    visit new_user_session_path

    # If user is already login, logout
    if page.has_text?("Agents")
      Capybara.reset_sessions!
      visit new_user_session_path
    end

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
    assert page.has_text?("Agent interpretations")
  end

end
