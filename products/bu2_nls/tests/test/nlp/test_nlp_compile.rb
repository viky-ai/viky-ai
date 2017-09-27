require 'test_helper'

module Nlp

  class TestNlpCompile < Common

    def input_ref()
      JSON.parse(File.read(fixture_path('package_base.json')))
    end

    def test_compile

      input = input_ref()

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end

      actual = nlp([ "-c #{@@pwd}/input.json"])

      expected = {"compilation"=>"ok"}

      assert_equal expected, actual
      File.delete("#{@@pwd}/input.json")
    end

    def test_dump
      input = input_ref

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end
      nlp([ "-c #{@@pwd}/input.json","-d","-o #{@@pwd}/output.json"])

      output = JSON.parse(File.read("#{@@pwd}/output.json"))

      assert_equal input, output
      File.delete("#{@@pwd}/output.json")
      File.delete("#{@@pwd}/input.json")
    end

    def test_intent_special_char
      input = input_ref()
      input[0]["intents"][0]["phrases"][0]["text"] =
      "@#!|\"\\\n_¢ß¥£™©®ª×÷±²³¼½¾µ¿¶·¸º°¯§…¤¦≠¬ˆ¨‰ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØŒŠþÙÚÛÜÝŸàáâãäåæçèéêëìíîïðñòóôõöøœšÞùúûüýÿ#自爾秦書わたしワタシ🎾"

      File.open("#{@@pwd}/input.json", 'w') do |f|
        f.write input.to_json
      end
      nlp([ "-c #{@@pwd}/input.json","-d","-o #{@@pwd}/output.json"])

      output = JSON.parse(File.read("#{@@pwd}/output.json"))

      assert_equal input, output
      File.delete("#{@@pwd}/output.json")
      File.delete("#{@@pwd}/input.json")

    end

  end

end
