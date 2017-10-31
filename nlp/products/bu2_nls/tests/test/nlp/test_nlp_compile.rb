require 'test_helper'

module Nlp

  class TestNlpCompile < NlpTestCommon

    def test_compile

      input_file = fixture_path('package_base.json')
      actual = nlp([ "-c #{input_file}"])

      expected = { "compilation" => "ok" }

      assert_equal expected, actual
    end

    def test_dump

      input_file = fixture_path('package_base.json')
      nlp([ "-c #{input_file}", "-d", "-o #{pwd}/output.json"])

      input  = JSON.parse(File.read(input_file))
      output = JSON.parse(File.read("#{pwd}/output.json"))

      assert_equal input, output
    end

    def test_intent_special_char

      input_file = fixture_path('package_specialchar.json')
      nlp([ "-c #{input_file}","-d","-o #{pwd}/output.json"])

      input  = JSON.parse(File.read(input_file))
      output = JSON.parse(File.read("#{pwd}/output.json"))

      assert_equal input, output

    end

  end

end
