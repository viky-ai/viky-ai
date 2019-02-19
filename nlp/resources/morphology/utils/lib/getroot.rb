#!/usr/bin/ruby
# frozen_string_literal: true
require 'fileutils'
class Getroot
  INPUT_REGEXP = /^(\D+)::(\D+)\+.*/.freeze

  # convert root line
  def process(line, type)
    m = INPUT_REGEXP.match(line)
    return nil unless m

    case type
    when :root
      word = m[1]
      root = m[2]
    when :form
      word = m[2]
      root = m[1]
    else
      "Unsupported dictionary type #{type}."
    end

    # word and root are equals
    return nil if word.nil?   || root.nil?
    return nil if root == '=' || word == '='
    return nil if word == root

    max_length = word.length
    root_word = word
    (0..max_length).each do |i|
      root_word = word[0, max_length - i]
      next unless !root_word.nil? && root.start_with?(root_word)

      # nb bytes to remove
      bytes_to_remove = word.bytesize - root_word.bytesize
      return "#{word}:#{bytes_to_remove}#{root.delete_prefix(root_word)}"
    end
    "#{word}:#{word.bytesize}#{root}"
  end

  # extract the root form from morphology files
  def generate_root_file(input_file_path, dictionary_type = :root)
    output_file_path = "#{input_file_path}.#{dictionary_type}"

    File.open(input_file_path, 'r') do |input_file|
      File.open(output_file_path, 'w') do |output_file|
        input_file.each_line do |line|
          line = line.strip
          next if line.empty?

          root = process(line, dictionary_type)
          output_file.write("#{root}\n") unless root.nil?
        end
      end
    end
  rescue
    FileUtils.mv(output_file_path, "#{output_file_path}.crashed")
    raise
  end
end
