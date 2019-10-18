# frozen_string_literal: true

require 'dotenv/load'

require 'minitest/autorun'
require 'minitest/reporters'

require 'active_support/all'
require 'rest-client'
require 'awesome_print'
require 'json'
require 'parallel'
require 'fileutils'
require 'uuidtools'

require 'time'

reporters = [Minitest::Reporters::SpecReporter.new]
ci_project_dir = ENV.fetch('CI_PROJECT_DIR', nil)
unless ci_project_dir.blank?
  reports_dir = "#{ci_project_dir}/reports/"
  FileUtils.mkdir_p reports_dir
  reporters << Minitest::Reporters::JUnitReporter.new(reports_dir, false)
end

backtrace_filter = Minitest::ExtensibleBacktraceFilter.default_filter
backtrace_filter.add_filter(/lib\/m/)
backtrace_filter.add_filter(/bin\/m/)
backtrace_filter.add_filter(/lib\/bundler/)
backtrace_filter.add_filter(/exe\/bundle/)
backtrace_filter.add_filter(/bin\/bundle/)
Minitest::Reporters.use!(reporters, ENV, backtrace_filter)

require 'nls'
require 'test_common'
require 'nls/nls_test_common'
require 'nlp/nlp_test_common'
