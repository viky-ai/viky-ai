# encoding: utf-8

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

reporters = [
  Minitest::Reporters::SpecReporter.new
]

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
