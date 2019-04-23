#!/usr/bin/env ruby
# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'getroot'

usage = 'getroot.rb frtotal root'
raise 'Input file is mandatory' if ARGV[0].nil?
raise 'Dictionary type (root|form) is mandatory' if ARGV[1].nil?

getroot = Getroot.new
getroot.generate_root_file(ARGV[0], ARGV[1].to_sym)
