# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestDates < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        package_number_digits = JSON.parse(File.read(fixture_path("package_number_digits.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_digits['id']}"
        Nls.query_post(package_url, package_number_digits)

        package_number_letters = JSON.parse(File.read(fixture_path("package_number_letters.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_letters['id']}"
        Nls.query_post(package_url, package_number_letters)

        package_number = JSON.parse(File.read(fixture_path("package_number.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number['id']}"
        Nls.query_post(package_url, package_number)

        package_date = JSON.parse(File.read(fixture_path("package_date.json")))
        package_url = "#{Nls.base_url}/packages/#{package_date['id']}"
        Nls.query_post(package_url, package_date)

      end


      def test_simple_date
        query = {
          "packages" =>
          [
            "package_number",
            "package_number_digits",
            "package_number_letters",
            "package_date"
          ],
          "sentence" => "3 décembre 2017"
        }


        expected = {
          "interpretations" => [
                  {
                       "package" => "package_date",
                            "id" => "date_day_month_year",
                          "slug" => "date_day_month_year",
                         "score" => 1.0,
                      "solution" => {
                          "date" => "2017-12-03T00:00:00.000Z"
                      }
                  }
              ]
        }
        actual = Nls.interpret(query)
        assert_equal expected, actual

#        query['sentence'] = "december the third, two thousand and seventeen"
#        actual = Nls.interpret(query)
#        ap actual
#        assert_equal expected, actual

        query['sentence'] = "december the third, 2017"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "december the 3, 2017"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "trois decembre deux mille dix sept"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "3/12/2017"
        actual = Nls.interpret(query)
        assert_equal expected, actual

      end

      def test_on_date
        query = {
          "packages" =>
          [
            "package_number",
            "package_number_digits",
            "package_number_letters",
            "package_date"
          ],
          "sentence" => "le 3 décembre 2017"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "package_date",
                            "id" => "single_date",
                          "slug" => "single_date",
                         "score" => 1.0,
                      "solution" => {
                          "date" => "2017-12-03T00:00:00.000Z"
                      }
                  }
              ]
        }

        query['sentence'] = "on 03/12/2017"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "le 3/12/2017"
        actual = Nls.interpret(query)
        assert_equal expected, actual

      end

      def test_interval_date

        query = {
          "packages" =>
          [
            "package_number",
            "package_number_digits",
            "package_number_letters",
            "package_date"
          ],
          "sentence" => "du 3 decembre 2017 au 15 janvier 2018"
        }
        expected = {
          "interpretations" => [
                  {
                       package: "package_date",
                            id: "date_interval",
                          slug: "date_interval",
                         score: 1.0,
                      solution: {
                          "date_begin" => "2017-12-03T00:00:00.000Z",
                          "date_end" => "2018-01-15T00:00:00.000Z"
                      }
                  }
              ]
        }
        actual = Nls.interpret(query)
        assert_json expected, actual

        query['sentence'] = "from december the 3, 2017 to january the 15, 2018"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "du trois decembre deux mille dix sept au quinze janvier deux mille dix huit"
        actual = Nls.interpret(query)
        assert_equal expected, actual

#        query['sentence'] = "from december the third, two thousand and seventeen to january the fifteenth, two thousand and eighteen"
#        actual = Nls.interpret(query)
#        assert_equal expected, actual

        query['sentence'] = "from 3/12/2017 to 15/01/2018"
        actual = Nls.interpret(query)
        assert_equal expected, actual

        query['sentence'] = "du 3/12/2017 au 15/01/2018"
        actual = Nls.interpret(query)
        assert_equal expected, actual

      end

    end
  end
end
