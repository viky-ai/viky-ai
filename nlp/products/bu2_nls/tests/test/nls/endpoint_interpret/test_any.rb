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

      def create_package(expression_with_number = false)
        package = Package.new("any_package")

        hello = package.new_interpretation("hello")
        hello << Expression.new("hello @{name}", aliases: { name: Alias.any })
        if expression_with_number
          hello << Expression.new("hello @{number} @{name}", aliases: { number: Alias.number, name: Alias.any })
          hello << Expression.new("hello @{name} @{number}", aliases: { number: Alias.number, name: Alias.any })
        end
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
        check_interpret("hello jean-marc", interpretation: "hello", solution: "jean-marc")
        check_interpret("hello ,jean-marc*", interpretation: "hello", solution: "jean-marc")
        check_interpret("hello ,brice, patrick,", interpretation: "hello", solution: "brice, patrick")
      end

      def test_any_punctuation_only
        check_interpret("hello ,+*", interpretation: "hello",  solution: nil)
      end

      def test_any_missing
        check_interpret("no_any",    interpretation: nil)
        check_interpret("no_any,+*", interpretation: nil)
      end

      def test_any_with_number

        expected = { interpretation: "hello", solution: "123,456.7 brice" }
        check_interpret("hello 123,456.7 brice",   expected)

        expected = { interpretation: "hello", solution: "brice 123,456.7" }
        check_interpret("hello brice 123,456.7",   expected)

      end

      def test_any_with_number_expression

        Nls.remove_all_packages
        Nls.package_update(create_package(true))

        expected = { interpretation: "hello", solution: { number: 123456.7, name: "brice" } }
        check_interpret("hello 123,456.7 brice",   expected)
        check_interpret("hello brice 123,456.7",   expected)

      end

    end
  end

end
