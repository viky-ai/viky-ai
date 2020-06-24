require "test_helper"
if ENV['COVERAGE']
  SimpleCov.command_name "rails-tests-system"
end
class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  include ActiveJob::TestHelper

  Capybara.register_driver(:headless_chrome) do |app|
    options = Selenium::WebDriver::Chrome::Options.new
    options.add_argument '--no-sandbox'
    options.add_argument '--disable-dev-shm-usage'
    options.add_argument '--disable-gpu'
    options.headless!
    driver_options = { browser: :chrome, options: options }
    driver_options[:url] = ENV['SELENIUM_REMOTE_URL'] if ENV['SELENIUM_REMOTE_URL']

    Capybara::Selenium::Driver.new(app, driver_options).tap do |driver|
      driver.browser.manage.window.size = Selenium::WebDriver::Dimension.new(1400, 900)
    end
  end

  Capybara.configure do |config|
    config.default_max_wait_time = 5
    config.automatic_label_click = true
  end

  driven_by :headless_chrome
  # driven_by :selenium, using: :chrome, screen_size: [1400, 1400]

  def setup
    return unless ENV['SELENIUM_REMOTE_URL']

    net = Socket.ip_address_list.detect(&:ipv4_private?)
    ip = net.nil? ? 'localhost' : net.ip_address
    Capybara.server_host = ip
    Capybara.always_include_port = true
    Capybara.app_host = "http://#{ip}"
  end

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

  def admin_go_to_agents_index
    admin_login
    assert page.has_text?("Agents")
  end

  def go_to_agent_interpretations(agent)
    visit user_agent_path(agent.owner, agent)
    assert page.has_link?('Interpretations')
    click_link 'Interpretations'
    within(".agent-header__content__nav .current") do
      assert page.has_text?('Interpretations')
    end
  end

  def go_to_agent_entities_lists(agent)
    visit user_agent_path(agent.owner, agent)
    assert page.has_link?('Entities')
    click_link 'Entities'
    within(".agent-header__content__nav .current") do
      assert page.has_text?('Entities')
    end
  end

  def go_to_agent_show(agent)
    visit user_agent_path(agent.owner, agent)
    assert page.has_text?('Overview')
  end

  def user_go_to_agent_show(user, agent)
    login_as user.email, 'BimBamBoom'
    go_to_agent_show(agent)
  end

  def admin_go_to_chatbots
    admin_login
    visit chatbots_path
    assert page.has_text?("Choose a chatbot on the left.")
  end

  def select_text_in_trix(selector, position_start, position_end)
    sleep(0.1)
    script = "$('#{selector}').first().trigger('select-text', [#{position_start},#{position_end}])"
    page.execute_script(script)
  end

  def assert_no_text_selected_in_trix(expression_id, text)
    script = "$('#expression-#{expression_id}')[0].textContent.search('<a .*>#{text}</a>')"
    result = page.evaluate_script(script)
    assert_equal -1, result
  end

  def assert_modal_is_close
    assert page.has_no_css?('.modal')
  end

  def build_fixture_files_path(filename)
    if ENV.key?('CI_PROJECT_DIR')
      webapp_root_path = File.join(ENV.fetch('CI_PROJECT_DIR'), 'webapp')
    else
      webapp_root_path = Rails.root
    end
    fixture_files_path = File.join(webapp_root_path, 'test', 'fixtures', 'files')
    File.join(fixture_files_path, filename)
  end
end
