require 'simplecov'
SimpleCov.start 'rails'

require File.expand_path('../../config/environment', __FILE__)
require 'rails/test_help'

require 'minitest/reporters'

reporters = [ Minitest::Reporters::DefaultReporter.new(color: true) ]
ci_project_dir = ENV.fetch('CI_PROJECT_DIR', nil)
unless ci_project_dir.blank?
  reports_dir = "#{ci_project_dir}/reports/"
  FileUtils.mkdir_p reports_dir
  reporters << Minitest::Reporters::JUnitReporter.new(reports_dir, false)
end
Minitest::Reporters.use!(reporters)

class ActiveSupport::TestCase
  make_my_diffs_pretty!
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

require 'mocha/minitest'
