require "test_helper"
SimpleCov.command_name "system"

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
    unless page.has_no_text?("Agents")
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

  def go_to_agent_intents(user, agent)
    admin_login
    visit user_agent_path(user, agent)
    assert page.has_link?('Interpretations')
    click_link 'Interpretations'
    within(".agent-header__content__nav .current") do
      assert page.has_text?('Interpretations')
    end
  end

  def go_to_agent_entities_lists(user, agent)
    admin_login
    visit user_agent_path(user, agent)
    assert page.has_link?('Entities lists')
    click_link 'Entities lists'
    within(".agent-header__content__nav .current") do
      assert page.has_text?('Entities lists')
    end
  end

  def go_to_agent_show(user, agent)
    admin_login
    visit user_agent_path(user, agent)
    assert page.has_text?("Overview")
  end

  def select_text_in_trix(selector, position_start, position_end)
    script = "$('#{selector}').first().trigger('select-text', [#{position_start},#{position_end}])"
    page.execute_script(script)
  end

  def assert_no_text_selected_in_trix(expression_id, text)
    script = "$('#expression-#{expression_id}')[0].textContent.search('<a .*>#{text}</a>')"
    result = page.evaluate_script(script)
    assert_equal -1, result
  end

end
