# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestWarning < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

      end

      def test_missing_package_warning

        missing_package = Package.new("missing_package")
        missing_paris = missing_package.new_interpretation("paris")
        missing_paris << Expression.new("Paris")

        package = Package.new("test_missing_package_warning")

        paris = package.new_interpretation("paris")
        paris << Expression.new("Paris")

        town = package.new_interpretation("town")
        town << Expression.new("@{paris}", aliases: { paris: missing_paris })


        # change interpretation id in package
        package_hash = package.to_h
        package_hash['interpretations'].last['expressions'].first['aliases'].first['id'] = paris.id.to_s

        Nls.package_update(package_hash)

        check_interpret("Paris", warnings: "Missing package '#{missing_package.id}'")
      end

      def test_missing_interpretation_warning
        missing_package = Package.new("missing_package")
        missing_paris = missing_package.new_interpretation("paris")
        missing_paris << Expression.new("Paris")

        package = Package.new("test_missing_package_warning")

        paris = package.new_interpretation("paris")
        paris << Expression.new("Paris")

        town = package.new_interpretation("town")
        town << Expression.new("@{paris}", aliases: { paris: missing_paris })

        Nls.package_update(package)

        check_interpret("Paris", warnings: "Missing interpretation '#{missing_paris.id}'")
      end

    end
  end

end
