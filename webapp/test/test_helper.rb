if ENV['COVERAGE']
  require 'simplecov'
  SimpleCov.command_name "rails-tests"
end
if ENV['RETRY']
  require 'minitest/retry'
  Minitest::Retry.use!
end
ENV['RAILS_ENV'] ||= 'test'
require_relative '../config/environment'
require 'rails/test_help'

if ENV['COVERAGE']
  ci_project_dir = ENV.fetch('CI_PROJECT_DIR', nil)
  unless ci_project_dir.blank?
    require 'minitest/reporters'
    reports_dir = "#{ci_project_dir}/reports/"
    FileUtils.mkdir_p reports_dir
    Minitest::Reporters.use!([
      Minitest::Reporters::DefaultReporter.new,
      Minitest::Reporters::JUnitReporter.new(reports_dir, false)
    ])
  end
end

class ActiveSupport::TestCase
  make_my_diffs_pretty!

  # Run tests in parallel with specified workers
  parallelize(workers: :number_of_processors)

  # Setup all fixtures in test/fixtures/*.yml for all tests in alphabetical order.
  fixtures :all

  Feature.disable_quota

  parallelize_setup do |worker|
    if ENV['COVERAGE']
      SimpleCov.command_name "#{SimpleCov.command_name}-#{worker}"
    end
    @index_client = InterpretRequestLogTestClient.new
    @index_client.create_test_index
  end

  setup do
    index_client = InterpretRequestLogTestClient.new
    index_client.clear_test_index
  end

  parallelize_teardown do
    if ENV['COVERAGE']
      SimpleCov.result
    end
    @index_client.drop_test_index
  end
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
