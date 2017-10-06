require 'test_helper'

module Nlp

  class TestNlpCompile < Common

    def input_ref()
      JSON.parse(File.read(fixture_path('package_base.json')))
    end

    def test_compile

      input = input_ref()

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{pwd}/input.json"])

      expected = {"compilation"=>"ok"}

      assert_equal expected, actual
    end

    def test_dump
      input = input_ref

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end
      nlp([ "-c #{pwd}/input.json","-d","-o #{pwd}/output.json"])

      output = JSON.parse(File.read("#{pwd}/output.json"))

      assert_equal input, output
    end

    def test_intent_special_char
      input = JSON.parse(File.read(fixture_path('package_specialchar.json')))

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end
      nlp([ "-c #{pwd}/input.json","-d","-o #{pwd}/output.json"])

      output = JSON.parse(File.read("#{pwd}/output.json"))

      assert_equal input, output

    end

  end

end
