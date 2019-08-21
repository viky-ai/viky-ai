# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestEnableList < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("enable_list")

        a = package.new_interpretation("a", { scope: "public" })
        a << Expression.new("aaa")

        b = package.new_interpretation("b", { scope: "public" })
        b << Expression.new("bbb")

        c = package.new_interpretation("c", { scope: "public" })
        c << Expression.new("ccc")

        ab = package.new_interpretation("ab", { scope: "public" })
        ab << Expression.new("aaa bbb", keep_order: true)

        package
      end

      def test_enable_list

        check_interpret("aaa", interpretations: ["a" ])
        check_interpret("aaa bbb", interpretations: ["ab" ])
        check_interpret("bbb aaa", interpretations: ["a", "b" ])
        check_interpret("aaa ccc", interpretations: ["a", "c" ])
        check_interpret("bbb aaa ccc", interpretations: ["a", "b", "c" ])
        check_interpret("aaa bbb ccc", interpretations: ["ab", "c" ])

      end

    end
  end

end
