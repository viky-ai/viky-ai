# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLocaleScore < NlsTestCommon

      def test_locale_score
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

        package_want_hotel_features = JSON.parse(File.read(fixture_path("package_want_hotel_features.json")))
        package_url = "#{Nls.base_url}/packages/#{package_want_hotel_features['id']}"
        Nls.query_post(package_url, package_want_hotel_features)

        package_want = JSON.parse(File.read(fixture_path("package_want.json")))
        package_url = "#{Nls.base_url}/packages/#{package_want['id']}"
        Nls.query_post(package_url, package_want)

        package_hotel = JSON.parse(File.read(fixture_path("package_hotel.json")))
        package_url = "#{Nls.base_url}/packages/#{package_hotel['id']}"
        Nls.query_post(package_url, package_hotel)

        query = {
          "packages":
          [
            "want_hotel_features",
            "package-want",
            "package-hotel",
            "hotel_features",
            "building_features",
            "number_people",
            "package-number",
            "package-number-digits",
            "package-number-letters"
          ],
          "sentence": "I want an hotel with a nice swimming pool with spa for 3 people and sea view",
          "Accept-Language": "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "want_hotel_features",
                            "id" => "want_hotel_features",
                          "slug" => "want_hotel_features",
                         "score" => 0.84,
                      "solution" => {
                          "book" => "hotel",
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
        assert_equal expected, actual

        query = {
          "packages":
          [
            "want_hotel_features",
            "package-want",
            "package-hotel",
            "hotel_features",
            "building_features",
            "number_people",
            "package-number",
            "package-number-digits",
            "package-number-letters"
          ],
          "sentence": "je veux un hotel avec piscine et spa pour 3 personnes et une vue sur la mer",
          "Accept-Language": "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "want_hotel_features",
                            "id" => "want_hotel_features",
                          "slug" => "want_hotel_features",
                         "score" => 0.90,
                      "solution" => {
                          "book" => "hotel",
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
        assert_equal expected, actual


      end
    end
  end
end
