# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestRegex < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("package")

        int1 = package.new_interpretation("regex", { scope: "public" })
        int1 << Expression.new("@{email}", aliases: { email: Alias.regex("[a-z]+@[a-z]+\.[a-z]+") })

        package
      end

      # Tests

      def test_regex_compile

        expected = {
          interpretation: "regex"
        }

        check_interpret("mail patrick@pertimm.com",        expected)
      end

    end
  end

end
