# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyMatch < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"
        package2 = create_package_2
        package1 = create_package_1(package2)

        Nls.package_update(package2)
        Nls.package_update(package1)
      end

      def create_package_1(pkg2)
        package = Package.new("package_1")

        int1 = package.new_interpretation("main_interpretation", { scope: "public" })
        int1 << Expression.new("@{number}", aliases: { number: Alias.number }, solution: "`{ number: number +1 }`")
        int1 << Expression.new("@{toto}", aliases: { toto: pkg2.interpretation("main") } )

        package
      end

      def create_package_2
        package2 = Package.new("package_2", { scope: "public" })

        int1 = package2.new_interpretation("main")
        int1 << Expression.new("@{number} @{toto}", aliases: { number: Alias.number, toto: Alias.any }, solution: "`{ number: number, toto: toto }`")

        package2
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
