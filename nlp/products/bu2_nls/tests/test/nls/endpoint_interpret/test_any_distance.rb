# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyDistance < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("any_distance")

        aaa = package.new_interpretation("aaa")
        aaa << Expression.new("aaa", solution: "aaa")

        bbb = package.new_interpretation("bbb")
        bbb << Expression.new("bbb", solution: "bbb")

        ccc = package.new_interpretation("ccc")
        ccc << Expression.new("ccc", solution: "ccc")

        ddd = package.new_interpretation("ddd")
        ddd << Expression.new("ddd", solution: "ddd")

        abcd = package.new_interpretation("abcd", { scope: "public" })
        abcd << Expression.new("@{aaa} @{bbb} @{ccc} @{ddd}", aliases: { aaa: aaa, bbb: bbb, ccc: Alias.any, ddd: ddd}, solution: "`{ ccc: ccc}`", keep_order: true, glue_distance: 5)

        package
      end

      def test_any_distance

        expected = { interpretation: "abcd", solution: { ccc: "ccc"} }
        check_interpret("aaa bbb ccc ddd", expected)

        expected = { interpretation: "abcd", solution: { ccc: "cccccccccc"} }
        check_interpret("aaa bbb cccccccccc ddd", expected)

      end


    end
  end

end
