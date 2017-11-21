require 'test_helper'

class PackageTest < ActiveSupport::TestCase

  test "package generation" do
    weather = agents(:weather)
    intent = intents(:weather_greeting)
    p = Nlp::Package.new(weather)

    expected = {
      "id"   => weather.id,
      "slug" => "admin/weather",
      "interpretations" => [
        {
          "id"   => intent.id,
          "slug" => "admin/weather/weather_greeting",
          "expressions" => [
            {
              "expression" => "Hello world",
              "locale"     => "en-US"
            },
            {
              "expression" => "Bonjour tout le monde",
              "locale"     => "fr-FR"
            }
          ]
        }
      ]
    }

    assert_equal expected, JSON.parse(p.generate_json)
  end

  test 'Validate endpoint' do
    weather = agents(:weather)
    p = Nlp::Package.new(weather)
    endpoint = ENV.fetch('VOQALAPP_NLP_URL') { 'http://localhost:9345' }
    assert_equal endpoint, p.endpoint
  end


  test 'Validate package URL' do
    weather = agents(:weather)
    p = Nlp::Package.new(weather)
    assert_equal "#{p.endpoint}/packages/#{weather.id}", p.url
  end
end
