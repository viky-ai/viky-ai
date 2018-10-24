# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyMatch < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("package")

        int1 = package.new_interpretation("sub_interpretation", { scope: "public" })
        int1 << Expression.new("@{number} @{toto}", aliases: { number: Alias.number, toto: Alias.any }, solution: "`{ number: number, toto: toto }`")

        int2 = package.new_interpretation("main_interpretation", { scope: "public" })
        int2 << Expression.new("@{toto}", aliases: { toto: int1 } )
        int2 << Expression.new("@{number}", aliases: { number: Alias.number }, solution: "`{ number: number +1 }`")

        package
      end

      # Tests

      def test_any_punctuation_trim

        expected = {
          "number_people" => 1235
        }

        check_interpret("1234",        expected)
      end

    end
  end

end
