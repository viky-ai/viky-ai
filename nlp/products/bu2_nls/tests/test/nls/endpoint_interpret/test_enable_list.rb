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

        abcd = package.new_interpretation("abcd", { scope: "public" })
        abcd << Expression.new("aaa", solution: "aaa")
        abcd << Expression.new("bbbb", solution: "bbbb")
        abcd << Expression.new("ccccc", solution: "ccccc")

        package
      end

      def test_enable_list

        expected = { interpretation: "abcd", solution: "aaa" }
        check_interpret("aaa bbbb cccc", expected)

      end


    end
  end

end
