require 'test_helper'

module Nlp

class TestNlpCompile < Minitest::Test

  @@pwd = ENV['NLS_INSTALL_PATH']
  @@pwd = "#{ENV['OG_REPO_PATH']}/ship/debug" if @@pwd.nil?

  def nlp(param = [])

    response = `cd #{@@pwd} && ./ognlp #{param.join(' ')}`
    JSON.parse response

  end

  def test_compile

    input =
      {
        "id"  => "voqal.ai:datetime",
        "intents"  => [
            {
              "id" => "0d981484-9313-11e7-abc4-cec278b6b50b",
              "phrases"  =>  [
                {
                  "text" => "Hello Brice",
                  "locale"  => "fr-FR"
                }
             ]
          }
        ]
      }


    File.open("#{@@pwd}/input.json", 'w') do |f|
      f.write input.to_json
    end

    actual = nlp([ "-c #{@@pwd}/input.json"])

    expected = {"compilation"=>"ok"}

    assert_equal expected, actual
  end



end

end
