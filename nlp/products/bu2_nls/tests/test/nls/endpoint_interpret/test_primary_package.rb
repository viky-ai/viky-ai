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
        @package_2 = Package.new("main")

        Nls.package_update(@package_1)
        Nls.package_update(@package_2)
      end

      def create_activity_package
        package = Package.new("activity")

        interpretation_ski = package.new_interpretation("i_ski", { scope: "private" })
        interpretation_ski << Expression.new("skier", solution: {activity: "ski"})

        interpretation_pour = package.new_interpretation("i_pour", { scope: "private" })
        interpretation_pour << Expression.new("pour")

        interpretation_public = package.new_interpretation("i_pub", { scope: "public" })
        interpretation_public << Expression.new("@{pour} @{activite}", aliases: {'pour' => interpretation_pour, 'activite' => interpretation_ski})
        interpretation_public << Expression.new("@{pour} @{activite}", aliases: {'pour' => interpretation_pour, 'activite' => Alias.any})

        package
      end

      def test_primary_package_deepness
        check_interpret("pour skier",
          packages: [ @package_1, @package_2 ],
          primary_package: @package_2,
          interpretation: nil
        )

        check_interpret("pour skier",
          packages: [ @package_1, @package_2 ],
          primary_package: @package_1,
          interpretation: "i_pub",
          solution: { activity: "ski" }
        )
      end


    end
  end

end
