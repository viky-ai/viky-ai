require 'simplecov'
SimpleCov.start 'rails'

ENV['RAILS_ENV'] ||= 'test'
require_relative '../config/environment'
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

  # Run tests in parallel with specified workers
  # parallelize(workers: :number_of_processors)

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

require 'mocha/minitest'
