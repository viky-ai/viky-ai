require 'simplecov'
SimpleCov.start 'rails'

require File.expand_path('../../config/environment', __FILE__)
require 'rails/test_help'

class ActiveSupport::TestCase
  # Setup all fixtures in test/fixtures/*.yml for all tests in alphabetical order.
  fixtures :all
end

class ActionDispatch::IntegrationTest
  include Devise::Test::IntegrationHelpers
end

require "shrine/storage/memory"

Shrine.storages = {
  cache: Shrine::Storage::Memory.new,
  store: Shrine::Storage::Memory.new,
}

require 'mocha/mini_test'

Capybara.register_driver :headless_chrome do |app|
  capabilities = Selenium::WebDriver::Remote::Capabilities.chrome(
    chromeOptions: {
      args: %w{headless no-sandbox disable-gpu window-size=1280,720}
    }
  )

  driver_options = {
    browser: :chrome,
    desired_capabilities: capabilities
  }
  if `cat /etc/os-release` =~ /ubuntu/i
    driver_options[:driver_path] = '/usr/lib/chromium-browser/chromedriver'
  end
  Capybara::Selenium::Driver.new(app, driver_options)
end
