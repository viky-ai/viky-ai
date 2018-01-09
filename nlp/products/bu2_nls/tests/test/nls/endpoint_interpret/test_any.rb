# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAny < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("any_package")

        hello = package.new_interpretation("hello")
        hello << Expression.new("hello @{name}", aliases: { name: Alias.any })

        package
      end

      # Tests

      def test_any_punctuation_trim
        check_interpret("hello brice,",  interpretation: "hello", solution: "brice")
        check_interpret("hello ,brice",  interpretation: "hello", solution: "brice")
        check_interpret("hello ,brice,", interpretation: "hello", solution: "brice")
        check_interpret("hello -brice.", interpretation: "hello", solution: "brice")
        check_interpret("hello *brice!", interpretation: "hello", solution: "brice")
      end

      def test_any_punctuation_trim_keep_inside
        check_interpret("hello ,brice, patrick,", interpretation: "hello", solution: "brice , patrick")
      end

      def test_any_punctuation_only
        # TODO bug punctuation should not match on any
        check_interpret("hello ,",  interpretation: "hello", solution: ",")
      end

    end
  end

end
