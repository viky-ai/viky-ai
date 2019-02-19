#!/usr/bin/env ruby
# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'getroot'

raise 'Input file is mandatory' if ARGV[0].nil?

getroot = Getroot.new
getroot.generate_root_file(ARGV[0], "#{ARGV[0]}.root")
