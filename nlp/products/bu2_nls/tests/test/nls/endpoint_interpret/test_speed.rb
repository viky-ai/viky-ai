# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestSpeed < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "fr"

        Nls.package_update(create_package())
      end

      def create_package()
        package = Package.new("speed_package")

        speed = package.new_interpretation("speed")
        speed << Expression.new("je sais compter jusqu'à @{count}", aliases: { count: Alias.number }, solution: "`{count: count}`")

        package
      end

      # Tests

      def test_count_until_10000
        10000.times do |i|
          expected = {
            count: i
          }
          check_interpret("je sais compter jusqu'à #{i}", interpretation: "speed", solution: expected)
        end
      end

    end


  end

end
