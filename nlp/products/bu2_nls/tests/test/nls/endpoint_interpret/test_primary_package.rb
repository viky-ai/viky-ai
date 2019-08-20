# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestPrimaryPackage < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr"

        @package_1 = create_activity_package
        @package_2 = create_activity_package2
        @package_3 = Package.new("main")

        Nls.package_update(@package_1)
        Nls.package_update(@package_2)
        Nls.package_update(@package_3)
      end

      def create_activity_package
        package = Package.new("activity")

        interpretation_ski = package.new_interpretation("i_ski", { scope: "private" })
        interpretation_ski << Expression.new("skier", solution: {activity: "ski"})

        interpretation_pour = package.new_interpretation("i_pour", { scope: "private" })
        interpretation_pour << Expression.new("pour")

        interpretation_public = package.new_interpretation("i_pub", { scope: "public" })
        interpretation_public << Expression.new("@{pour} @{activite}", aliases: {'pour' => interpretation_pour, 'activite' => interpretation_ski})

        package
      end

    def create_activity_package2
      package = Package.new("activity2")

      interpretation_marche = package.new_interpretation("i_marche", { scope: "private" })
      interpretation_marche << Expression.new("marcher", solution: {activity: "marche"})

      interpretation_pour = package.new_interpretation("i_pour", { scope: "private" })
      interpretation_pour << Expression.new("pour")

      interpretation_public = package.new_interpretation("i_pub2", { scope: "public" })
      interpretation_public << Expression.new("@{pour} @{activite}", aliases: {'pour' => interpretation_pour, 'activite' => interpretation_marche})

      package
    end

    def test_primary_package_deepness
      check_interpret("pour skier",
        packages: [ @package_1, @package_3 ],
        show_private: true,
        primary_package: @package_3,
        interpretation: "i_pub",
        solution: { activity: "ski" }
      )
    end

    def test_primary_packages
      expected = {
        packages: [ @package_1, @package_2, @package_3 ],
        primary_packages: [ @package_1, @package_2],
        interpretations: [ "i_pub" , "i_pub2" ]
      }
      check_interpret( "pour marcher, pour skier",  expected)
    end


    end
  end

end
