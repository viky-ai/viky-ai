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
        abcd << Expression.new("bbb", solution: "bbb")
        abcd << Expression.new("ccc", solution: "ccc")
        abcd << Expression.new("aaa bbb", solution: "aaa bbb")

        package
      end

      def test_enable_list

        expected = { interpretation: "abcd", solution: "aaa bbb" }
        check_interpret("aaa bbb", expected)

        #enable_list option to be implemented and check_interpret_list also (not check_interpret_array).
        expected = [{ interpretation: "abcd", solution: "aaa" },{ interpretation: "abcd", solution: "ccc" }]
        check_interpret({sentence: "aaa ccc", enable_list: true}, expected)

      end


    end
  end

end
