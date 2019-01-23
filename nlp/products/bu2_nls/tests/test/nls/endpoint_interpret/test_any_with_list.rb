# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyWithList < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("any_with_list_package")

        i_want = package.new_interpretation("want", { scope: "private" })
        i_want << Expression.new("I want")

        i_go = package.new_interpretation("go", { scope: "private" })
        i_go << Expression.new("to go")

        i_voc_to = package.new_interpretation("voc_to", { scope: "private" })
        i_voc_to << Expression.new("to")

        i_location = package.new_interpretation("location", { scope: "private" })
        i_location << Expression.new("Paris", solution: 'Paris')

        i_to_location = package.new_interpretation("to_location", { scope: "private" })
        i_to_location << Expression.new("@{voc_to} @{location}", aliases: { voc_to: i_voc_to, location: i_location })
        i_to_location << Expression.new("@{voc_to} @{location}", aliases: { voc_to: i_voc_to, location: Alias.any() })

        i_activity = package.new_interpretation("activity", { scope: "private" })
        i_activity << Expression.new("jet ski", solution: 'jet_ski')

        i_hotel_feature = package.new_interpretation("hotel_feature", { scope: "private" })
        i_hotel_feature << Expression.new("@{to_location}", aliases: {to_location: i_to_location})
        i_hotel_feature << Expression.new("@{activity}", aliases: {activity: i_activity})

        i_hotel_features = package.new_interpretation("hotel_features", { scope: "private" })
        i_hotel_features << Expression.new("@{hotel_feature}", aliases: {hotel_feature: i_hotel_feature })
        i_hotel_features << Expression.new("@{hotel_feature} @{hotel_features}", aliases: {hotel_feature: i_hotel_feature, hotel_features: i_hotel_features })

        i_book_hotel = package.new_interpretation("book_hotel", { scope: "public" })
        i_book_hotel << Expression.new("@{go} @{hotel_features}", aliases: { go: i_go, hotel_features: i_hotel_features })
        i_book_hotel << Expression.new("@{go} @{to_location}", aliases: { go: i_go, to_location: i_to_location })

        package
      end

      # Tests

      def test_any_with_list

        expected = { interpretation: "book_hotel", solution: {"hotel_features"=>[{"location"=>"tombouctou"}]} }
        check_interpret("to go to tombouctou",        expected)
        check_interpret("I want to go to tombouctou", expected)

      end

    end
  end

end
