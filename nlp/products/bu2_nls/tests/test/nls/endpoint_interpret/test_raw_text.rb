# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestRawTextMatch < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr"

        Nls.package_update(create_package)
      end

       def create_package
        package = Package.new("package")

         int_petit = package.new_interpretation("petit", { scope: "public" })
         int_petit << Expression.new("petit", solution: "petit")

         int_blanc = package.new_interpretation("blanc", { scope: "public" })
         int_blanc << Expression.new("blanc", solution: "blanc")

         int_cheval = package.new_interpretation("cheval", { scope: "public" })
         int_cheval << Expression.new("cheval", solution: "`{ raw: raw_text }`")

         int_petit_blanc = package.new_interpretation("petit_blanc", { scope: "public" })
         int_petit_blanc << Expression.new("@{petit} @{blanc}", aliases: { petit: int_petit, blanc: int_blanc }, solution: "`{ petit: petit, blanc: blanc, my_raw: raw_text }`")

        package
      end

      # Tests

      def test_cheval
        expected = { interpretation: "cheval", solution: { raw: "Cheval" } }
        check_interpret("Cheval",        expected)
      end

      def test_petit_blanc
        expected = { interpretation: "petit_blanc", solution: { petit: "petit", blanc: "blanc", my_raw: "petit verre blanc" } }
        check_interpret("un petit verre blanc",        expected)
      end

    end
  end

end
