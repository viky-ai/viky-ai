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

        query = {
          "packages" =>
          [
            "package_number",
            "package_number_digits",
            "package_number_letters"
          ],
          "sentence" => "3"
        }

        expected = {
          "interpretations" => [
                  {
                       "package" => "package_number",
                            "id" => "number",
                          "slug" => "number",
                         "score" => 1.0,
                      "solution" => {
                         "number" => 3
                      }
                  }
              ]

        }

        query_param = { }

        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "4"
        expected["interpretations"][0]["solution"]["number"] = 4
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "51"
        expected["interpretations"][0]["solution"]["number"] = 51
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "fifty two"
        expected["interpretations"][0]["solution"]["number"] = 52
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "two hundred and fifty two"
        expected["interpretations"][0]["solution"]["number"] = 252
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "quarante et une"
        expected["interpretations"][0]["solution"]["number"] = 41
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "quatre vingt douze"
        expected["interpretations"][0]["solution"]["number"] = 92
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "sept cent dix huit mille quatre cent quatre vingt quatorze"
        expected["interpretations"][0]["solution"]["number"] = 718494
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "quarante douze"
        expected["interpretations"][0]["solution"]["number"] = 52
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        expected = {
          "interpretations" => [
                  {
                       "package" => "package_number",
                            "id" => "number_ordinal",
                          "slug" => "number_ordinal",
                         "score" => 1.0,
                      "solution" => {
                         "number" => 3
                      }
                  }
              ]

        }

        query['sentence'] = "troisieme"
        expected["interpretations"][0]["solution"]["number"] = 3
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

        query['sentence'] = "third"
        expected["interpretations"][0]["solution"]["number"] = 3
        actual = Nls.interpret(query, query_param)
        assert_json expected["interpretations"].first, actual["interpretations"].first

      end
    end
  end
end
