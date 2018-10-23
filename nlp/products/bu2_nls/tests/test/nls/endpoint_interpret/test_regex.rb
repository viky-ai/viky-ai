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

        email = package.new_interpretation("email", { scope: "private" })
        email << Expression.new("@{email}", aliases: { email: Alias.regex("[a-z]+@[a-z]+\.[a-z]+") })

        emails = package.new_interpretation("emails", { scope: "public" })
        emails << Expression.new("@{email}", aliases: { email: email })
        emails << Expression.new("@{email} @{emails}", aliases: { email: email, emails: emails })

        package
      end

      # Tests

      def test_regex_compile

        expected = {
          interpretation: "emails"
        }

        check_interpret("mail patrick@pertimm.com",        expected)
        check_interpret("mail patrick@pertimm.com sebastien@pertimm.com",        expected)
      end

    end
  end

end
