require "test_helper"
SimpleCov.command_name "system"

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  include ActiveJob::TestHelper

  driven_by :selenium, using: :headless_chrome, screen_size: [1400, 900]

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

  def go_to_agent_intents(agent)
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


end
