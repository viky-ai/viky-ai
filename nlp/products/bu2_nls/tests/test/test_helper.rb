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
  Minitest::Reporters::SpecReporter.new( color:true),
  # Minitest::Reporters::HtmlReporter.new(reports_dir: 'html_reports')
]

Minitest::Reporters.use! reporters

require 'test_common'
require 'nls/common'
require 'nlp/common'
require 'nls/nls'
