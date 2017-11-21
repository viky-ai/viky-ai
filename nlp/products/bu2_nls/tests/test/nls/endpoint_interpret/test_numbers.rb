# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNumbers < NlsTestCommon

      def test_simple_numbers
        Nls.remove_all_packages

        package_number_digits = JSON.parse(File.read(fixture_path("package_number_digits.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_digits['id']}"
        Nls.query_post(package_url, package_number_digits)

        package_number_letters = JSON.parse(File.read(fixture_path("package_number_letters.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_letters['id']}"
        Nls.query_post(package_url, package_number_letters)

        package_number = JSON.parse(File.read(fixture_path("package_number.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number['id']}"
        Nls.query_post(package_url, package_number)

        package_number_people = JSON.parse(File.read(fixture_path("package_number_people.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_people['id']}"
        Nls.query_post(package_url, package_number_people)

        package_hotel_features = JSON.parse(File.read(fixture_path("package_hotel_features.json")))
        package_url = "#{Nls.base_url}/packages/#{package_hotel_features['id']}"
        Nls.query_post(package_url, package_hotel_features)

        package_building_features = JSON.parse(File.read(fixture_path("package_building_features-v2.json")))
        package_url = "#{Nls.base_url}/packages/#{package_building_features['id']}"
        Nls.query_post(package_url, package_building_features)

        query = {
          "packages" =>
          [
            "hotel_features",
            "building_features",
            "number_people",
            "package-number",
            "package-number-digits",
            "package-number-letters"
          ],
          "sentence" => "with swimming pool with spa for 3 people with sea view"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "hotel_features",
                            "id" => "hotel_features",
                          "slug" => "hotel_features",
                         "score" => 1.0,
                      "solution" => {
                          "hotel_feature" => [
                              "spa",
                              "swimming_pool",
                              "sea_view"
                          ],
                          "number-people" => 3.0
                      }
                  }
              ]

        }

        # TODO fix test timeout
        query_param = { timeout: 2000 }

        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "with swimming pool with spa for 4 people with sea view"
        expected["interpretations"][0]["solution"]["number-people"] = 4.0
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "with swimming pool with spa for 51 people with sea view"
        expected["interpretations"][0]["solution"]["number-people"] = 51
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "with swimming pool with spa for fifty two people with sea view"
        expected["interpretations"][0]["solution"]["number-people"] = 52
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "with swimming pool with spa for two hundred and fifty two people with sea view"
        expected["interpretations"][0]["solution"]["number-people"] = 252
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "avec piscine avec spa pour quarante et une personnes avec vue sur la mer"
        expected["interpretations"][0]["solution"]["number-people"] = 41
        actual = Nls.interpret(query, query_param)

        query['sentence'] = "avec piscine avec spa pour quatre vingt douze personnes avec vue sur la mer"
        expected["interpretations"][0]["solution"]["number-people"] = 92
        actual = Nls.interpret(query, query_param)

        query['sentence'] = "avec piscine avec spa pour sept cent dix huit mille quatre cent quatre vingt quatorze personnes avec vue sur la mer"
        expected["interpretations"][0]["solution"]["number-people"] = 718494
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

        query['sentence'] = "avec piscine avec spa pour quarante douze personnes avec vue sur la mer"
        expected["interpretations"][0]["solution"]["number-people"] = 52
        actual = Nls.interpret(query, query_param)
        assert_equal expected, actual

      end
    end
  end
end
