require 'test_helper'

module Nlp

  class TestNlpErrors < NlpTestCommon

    def input_ref()
      JSON.parse(File.read(fixture_path('package_base.json')))
    end

    def test_package_id_type

      input = input_ref

      input[0]["id"] = 123

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{pwd}/input.json"])

      expected_error = "package 'id' is not a string"

      assert_response_has_error  expected_error, actual
    end

    def test_intent_id_type

      input = input_ref
      input[0]["interpretations"][0]["id"] = 123

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{pwd}/input.json"])

      expected_error = "interpretation 'id' is not a string"

      assert_response_has_error  expected_error, actual
    end

    def test_intent_locale

      input = input_ref
      input[0]["interpretations"][0]["expressions"][0]["locale"] = "ok-KO"

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{pwd}/input.json"])

      expected_error = "OgCodeToIso639_3166: Language 'ok' does not exist"

      assert_response_has_error expected_error, actual
    end

    def test_intent_phrase_too_long

      input = input_ref
      input[0]["interpretations"][0]["expressions"][0]["expression"] = ""

      500.times do
        input[0]["interpretations"][0]["expressions"][0]["expression"]  << "Brice Hello "
      end

      File.open("#{pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{pwd}/input.json"])

      expected_error = "NlpCompilePackageExpression: expression text is too long"

      assert_response_has_error expected_error, actual
    end

  end

end
