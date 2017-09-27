require 'test_helper'

module Nlp

  class TestNlpErrors < Common

    def input_ref()
      JSON.parse(File.read(fixture_path('package_base.json')))
    end

    def test_package_id_type

      input = input_ref

      input[0]["id"] = 123

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{@@pwd}/input.json"])

      expected = {"errors" => ["NlpCompilePackageIntents: id is not a string"]}

      assert_equal expected, actual
      File.delete("#{@@pwd}/input.json")
    end

    def test_intent_id_type

      input = input_ref
      input[0]["intents"][0]["id"] = 123

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{@@pwd}/input.json"])

      expected = {"errors" => ["NlpCompilePackageIntent: id is not a string"]}

      assert_equal expected, actual
      File.delete("#{@@pwd}/input.json")
    end

    def test_package_intents_type

      input = input_ref

      actual = Array
      expected = input[0]["intents"].class

      assert_equal expected, actual
    end

    def test_intent_locale

      input = input_ref
      input[0]["intents"][0]["sentences"][0]["locale"] = "ok-KO"

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{@@pwd}/input.json"])

      expected = {"errors" => ["OgCodeToIso639_3166: Language 'ok' does not exist"]}

      assert_equal expected, actual
      File.delete("#{@@pwd}/input.json")
    end

    def test_intent_phrase_too_long

      input = input_ref
      input[0]["intents"][0]["sentences"][0]["sentence"] = ""

      500.times do
        input[0]["intents"][0]["sentences"][0]["sentence"]  << "Brice Hello "
      end

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{@@pwd}/input.json"])

      expected = {"errors" => ["NlpCompilePackageSentence: text is too long"]}

      assert_equal expected, actual
      File.delete("#{@@pwd}/input.json")
    end

  end

end
