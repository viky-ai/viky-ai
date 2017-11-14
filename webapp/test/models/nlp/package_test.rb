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

end
