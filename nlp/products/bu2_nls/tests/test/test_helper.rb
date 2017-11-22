# encoding: utf-8

require 'dotenv/load'

require 'minitest/autorun'
require 'minitest/reporters'

require 'rest-client'
require 'awesome_print'
require 'json'
require 'parallel'
require 'fileutils'
require 'uuidtools'

reporters = [
  Minitest::Reporters::SpecReporter.new(color: true)
]

Minitest::Reporters.use! reporters

require 'nls'
require 'test_common'
require 'nls/nls_test_common'
require 'nlp/nlp_test_common'
