# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAccentCase < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("accent_case")

        car = package.new_interpretation("villes", { scope: "public" })
        car << Expression.new("Bourg-la-Reine", solution: "Bourg-la-Reine", keep_order: true, glue_distance: 0, glue_strength: "punctuation")
        car << Expression.new("Placé", solution: "Placé", keep_order: true, glue_distance: 0, case_sensitive: true, accent_sensitive: true)
        car << Expression.new("Nouméa", solution: "Nouméa", keep_order: true, glue_distance: 0, case_sensitive: true, accent_sensitive: false)
        car << Expression.new("Béziers", solution: "Béziers", keep_order: true, glue_distance: 0, case_sensitive: false, accent_sensitive: true)

        package
      end


      def test_case_accent
        expected = { interpretation: "villes", solution: "Bourg-la-Reine" }
        check_interpret("bourg la reine", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("place", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "villes", solution: "Nouméa" }
        check_interpret("Nouméa", expected)

        expected = { interpretation: "villes", solution: "Nouméa" }
        check_interpret("Noumea", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("nouméa", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "villes", solution: "Béziers" }
        check_interpret("Béziers", expected)

        expected = { interpretation: "villes", solution: "Béziers" }
        check_interpret("béziers", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("Beziers", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

      end

    end
  end

end
