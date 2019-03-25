# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestDump < NlsTestCommon

      def test_nls_dump
        Nls.remove_all_packages

        package = Package.new("package")
        i_first_interpretation = Interpretation.new("interpretation_1", { solution: { agaga: "ceci est une solution"}})
        i_first_interpretation.new_textual("mytextual", {
                                                          keep_order: Expression.keep_order,
                                                          glue_distance: Expression.glue_distance,
                                                          solution: {"to" => "`town`"}
                                                        })
        package << i_first_interpretation

        i_second_interpretation_params = {
                aliases: { first: i_first_interpretation, any: Alias.any, numbers: Alias.number},
                keep_order: Expression.keep_order,
                solution: { to: "`town.name`"}
              }
        i_second_interpretation = Interpretation.new("interpretation_2", solution: { deuze:  "ceci est une autre solution",
                                                                                     troize: "et de 3! "
                                                                                   } )
        i_second_interpretation.new_expression("@expression_1", i_second_interpretation_params)
        package << i_second_interpretation

        Nls.package_update(package)
        actual = Nls.dump

        assert_equal package.to_h, actual[0]

      end

      def test_nls_dump_several_complex_packages
        Nls.remove_all_packages

        package_number_numbers = JSON.parse(File.read(fixture_path("package_number_digits.json")))
        package_url = "#{Nls.base_url}/packages/#{package_number_numbers['id']}"
        Nls.query_post(package_url, package_number_numbers)

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

        10.times do |i|
          actual_dump_result = Nls.dump
          assert !actual_dump_result.nil?
        end


      end


    end
  end
end
