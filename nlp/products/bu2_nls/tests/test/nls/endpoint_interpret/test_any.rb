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
        hello << Expression.new("hello")

        no_any = package.new_interpretation("no_any")
        no_any << Expression.new("no_any @{missing}", aliases: { missing: Alias.any })

        package
      end

      # Tests

      def test_any_punctuation_trim

        expected = { interpretation: "hello", solution: "brice" }

        check_interpret("hello brice",        expected)
        check_interpret("hello brice,",       expected)
        check_interpret("hello ,brice",       expected)
        check_interpret("hello ,brice,",      expected)
        check_interpret("hello -brice.",      expected)
        check_interpret("hello :;*brice!+!/", expected)
        check_interpret("hello*brice*",       expected)

      end

      def test_any_punctuation_trim_keep_inside
        check_interpret("hello ,brice, patrick,", interpretation: "hello", solution: "brice , patrick")
      end

      def test_any_punctuation_only
        check_interpret("hello ,+*", interpretation: "hello",  solution: nil)
      end

      def test_any_missing
        check_interpret("no_any",    interpretation: nil)
        check_interpret("no_any,+*", interpretation: nil)
      end

    end
  end

end
