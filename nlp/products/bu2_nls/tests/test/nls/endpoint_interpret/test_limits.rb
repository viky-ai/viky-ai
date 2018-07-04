# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLimits < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

      end



      def test_deep_any

        package = Package.new("deep_any")

        horse = package.new_interpretation("horse")
        horse << Expression.new("horse")

        tiger = package.new_interpretation("tiger")
        tiger << Expression.new("tiger")

        animals = package.new_interpretation("animals")
        animals << Expression.new("@{horse}",  aliases: { horse: horse})
        animals << Expression.new("@{tiger}",  aliases: { tiger: tiger})

        whatever = package.new_interpretation("whatever")
        whatever << Expression.new("@{whatever}",  aliases: { whatever: Alias.any})

        whatever_animal_good = package.new_interpretation("whatever_animal_good")
        whatever_animal_good << Expression.new("@{whatever} @{animal}",  aliases: { whatever: Alias.any, animal: animals})

        whatever_animal_bad = package.new_interpretation("whatever_animal_bad")
        whatever_animal_bad << Expression.new("@{whatever} @{animal}",  aliases: { whatever: whatever, animal: animals})

        Nls.package_update(package)

        check_interpret("black horse", warnings: "Inactive alias of type any 'whatever'")

      end


      def test_double_alias

        package = Package.new("double_alias")

        horse = package.new_interpretation("horse")
        horse << Expression.new("horse")

        tiger = package.new_interpretation("tiger")
        tiger << Expression.new("tiger")

        animals = package.new_interpretation("animals")
        animals << Expression.new("@{horse}",  aliases: { horse: horse})
        animals << Expression.new("@{tiger}",  aliases: { tiger: tiger})

        whatever_animal_good = package.new_interpretation("double_alias")
        whatever_animal_good << Expression.new("@{animal} @{animal}",  aliases: { animal: animals})

        Nls.package_update(package)

        check_interpret("tiger horse", interpretation: "double_alias")

      end

      def test_limit_algo

        package = Package.new("limit_algo")

        slow = package.new_interpretation("slow")
        slow << Expression.new("slow")
        slow << Expression.new("swimming slow")

        swimming_for_relaxing = package.new_interpretation("swimming_for_relaxing")
        swimming_for_relaxing << Expression.new("swimming for relaxing")
        swimming_for_relaxing << Expression.new("relaxing swimming")

        animals = package.new_interpretation("slow_swimming_relaxing")
        animals << Expression.new("@{slow} @{swimming_for_relaxing}",  aliases: { slow: slow, swimming_for_relaxing: swimming_for_relaxing})

        Nls.package_update(package)

        check_interpret("slow swimming for relaxing", interpretation: "slow_swimming_relaxing")

      end




    end
  end
end
