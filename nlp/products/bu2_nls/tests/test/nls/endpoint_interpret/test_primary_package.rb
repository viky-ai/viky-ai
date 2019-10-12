# frozen_string_literal: true

require 'test_helper'

module Nls
  module EndpointInterpret
    class TestPrimaryPackage < NlsTestCommon
      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = 'fr'

        @package_activity1 = create_activity1_package
        @package_activity2 = create_activity2_package
        @package_main = Package.new('main')

        Nls.package_update(@package_activity1)
        Nls.package_update(@package_activity2)
        Nls.package_update(@package_main)
      end

      def create_activity1_package
        package = Package.new('activity1')

        interpretation_ski = package.new_interpretation('i_ski', scope: 'private')
        interpretation_ski << Expression.new('skier', solution: { activity: 'ski' })

        interpretation_pour = package.new_interpretation('i_pour', scope: 'private')
        interpretation_pour << Expression.new('pour')

        interpretation_public = package.new_interpretation('i_pub', scope: 'public')
        aliases = { 'pour' => interpretation_pour, 'activite' => interpretation_ski }
        interpretation_public << Expression.new('@{pour} @{activite}', aliases: aliases, glue_distance: 0)

        package
      end

      def create_activity2_package
        package = Package.new('activity2')

        interpretation_marche = package.new_interpretation('i_marche', scope: 'private')
        interpretation_marche << Expression.new('marcher', solution: { activity: 'marche' })

        interpretation_pour = package.new_interpretation('i_pour', scope: 'private')
        interpretation_pour << Expression.new('pour')

        interpretation_public = package.new_interpretation('i_pub2', scope: 'public')
        aliases = { 'pour' => interpretation_pour, 'activite' => interpretation_marche }
        interpretation_public << Expression.new('@{pour} @{activite}', aliases: aliases, glue_distance: 0)

        package
      end

      def test_primary_package_deepness
        expected = {
          packages: [@package_activity1, @package_main],
          primary_package: @package_main,
          interpretation: nil
        }
        check_interpret('pour skier', expected)
      end

      def test_primary_packages
        expected = {
          packages: [@package_activity1, @package_activity2, @package_main],
          primary_packages: [@package_activity1, @package_activity2],
          interpretation: 'i_pub2'
        }
        check_interpret('pour marcher', expected)

        expected = {
          packages: [@package_activity1, @package_activity2, @package_main],
          primary_packages: [@package_activity1, @package_activity2],
          interpretations: %w[i_pub i_pub2]
        }
        check_interpret('pour marcher, pour skier', expected)
      end
    end
  end
end
