require 'simplecov'
SimpleCov.start 'rails'

require File.expand_path('../../config/environment', __FILE__)
require 'rails/test_help'

class ActiveSupport::TestCase
  make_my_diffs_pretty!
  fixtures :all

  Feature.disable_rack_throttle
end

class ActionDispatch::IntegrationTest
  include Devise::Test::IntegrationHelpers
end

require "shrine/storage/memory"

Shrine.storages = {
  cache: Shrine::Storage::Memory.new,
  store: Shrine::Storage::Memory.new,
}

require 'mocha/minitest'
