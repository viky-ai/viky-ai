# encoding: utf-8

require 'minitest/autorun'
require 'minitest/reporters'

require 'rest-client'
require 'awesome_print'
require 'json'

Minitest::Reporters.use! Minitest::Reporters::SpecReporter.new # spec-like progress

require 'common'
