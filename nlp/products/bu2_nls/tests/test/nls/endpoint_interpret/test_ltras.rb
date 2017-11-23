# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLtras < NlsTestCommon

      def test_ltras_single_words
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

        # swimmming, pol, peple and viiaw are corrected
        # viiaw is corrected by the ltras phon module
        query = {
          packages:
          [
            "hotel_features",
            "building_features",
            "number_people",
            "package_number",
            "package_number_digits",
            "package_number_letters"
          ],
          sentence: "with a nice swimmming pol with spa for 3 peple and sea viiaw"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "hotel_features",
                            "id" => "hotel_features",
                          "slug" => "hotel_features",
                         "score" => 0.91,
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

        query_param = { timeout: 2000 }
        actual = Nls.interpret(query, query_param)
        assert_json expected, actual

      end
    end
  end
end
