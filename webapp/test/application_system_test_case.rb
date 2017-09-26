require "test_helper"
require 'capybara/poltergeist'

class ApplicationSystemTestCase < ActionDispatch::SystemTestCase
  driven_by :poltergeist

  def admin_login
    visit new_user_session_path
    fill_in 'Email', with: 'admin@voqal.ai'
    fill_in 'Password', with: 'AdminBoom'
    click_button 'Log in'
  end
end
