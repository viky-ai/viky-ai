require 'test_helper'



class TestNlpCompile < Minitest::Test

  @@pwd = ENV['NLS_INSTALL_PATH']
  @@pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if @@pwd.nil?

  def nlp(param = [])

    response = `cd #{@@pwd} && ./ognlp #{param.join(' ')}`
    JSON.parse response

  end

  def test_compile

    input = {
     "toto": 1
    }

    File.open("#{@@pwd}/input.json", 'w') do |f|
      f.write input.to_json
    end

    actual = nlp([ "-c #{@@pwd}/input.json"])

    expected = {"toto"=>"titi"}

    assert_equal expected, actual
  end



end
