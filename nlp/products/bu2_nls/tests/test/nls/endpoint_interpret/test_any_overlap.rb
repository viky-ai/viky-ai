# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyOverlap < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

       def create_package
        package = Package.new("package")


        ram_size = package.new_interpretation("ram_size")
        ram_size << Expression.new("@{number} GO RAM", aliases: { number: Alias.number}, solution: "`{ size: number}`")

        int2 = package.new_interpretation("main_interpretation", { scope: "public" })
        int2 << Expression.new("ordinateur @{brand} @{ram_size}", aliases: { brand: Alias.any, ram_size: ram_size }, solution: "`{ brand: brand, size: ram_size.size}`")

        package
      end

      # Tests

      def test_any_overlap
        expected = {
          solution: {
            "brand" => "dell",
            "size" => 12,
          }
        }

        check_interpret("je veux un ordinateur dell avec 12 Go de RAM", expected)
      end

    end
  end

end
