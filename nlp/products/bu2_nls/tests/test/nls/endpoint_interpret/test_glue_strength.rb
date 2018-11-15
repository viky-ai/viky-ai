# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestGlueStrength < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("glue_strength")

        car = package.new_interpretation("car", { scope: "public" })
        car << Expression.new("bmw-serie-4", solution: "bmw_serie_4", keep_order: true, glued: true, glue_strength: "punctuation")
        car << Expression.new("audi a4", solution: "audi_a4", keep_order: true, glued: true)

        package
      end


      def test_glue_strength
        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW serie 4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW-serie 4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW-serie-4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW_serie_4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW'serie'4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW/serie/4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW–serie–4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW’serie’4", expected)

        expected = { interpretation: "car", solution: "audi_a4" }
        check_interpret("Audi A4", expected)

        expected = { interpretation: "car", solution: "bmw_serie_4" }
        check_interpret("BMW--serie---4", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("Audi-A4", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        exception = assert_raises Minitest::Assertion do
          check_interpret("BMW&serie&4", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

      end

    end
  end

end
