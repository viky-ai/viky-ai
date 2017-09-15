# encoding: utf-8

require 'dotenv/load'

require 'minitest/autorun'
require 'minitest/reporters'

require 'rest-client'
require 'awesome_print'
require 'json'
require 'parallel'

Minitest::Reporters.use! Minitest::Reporters::SpecReporter.new # spec-like progress

require 'common'
require 'nls'
