# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestKeepOrder < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("keep_order_package")

        titi = package.new_interpretation("titi", scope: "private")
        titi << Expression.new("titi", solution: "titi")

        toto = package.new_interpretation("toto", scope: "private")
        toto << Expression.new("toto", solution: "toto")

        tutu = package.new_interpretation("tutu", scope: "private")
        tutu << Expression.new("tutu", solution: "tutu")

        tata = package.new_interpretation("tata", scope: "private")
        tata << Expression.new("tata", solution: "tata")

        mixed_no_keep_order = package.new_interpretation("mixed_no_keep_order", scope: "public")
        mixed_no_keep_order << Expression.new("@{titi} @{toto} @{tutu}", aliases: {titi: titi, toto: toto, tutu: tutu}, solution: {titi: "titi", toto: "toto", tutu: "tutu"})

        mixed_keep_order = package.new_interpretation("mixed_keep_order", scope: "public")
        mixed_keep_order << Expression.new("@{titi} @{tata} @{toto}", keep_order: true, aliases: {titi: titi, tata: tata, toto: toto}, solution: {titi: "titi", tata: "tata", toto: "toto"})

        mixed_no_keep_order_glued = package.new_interpretation("mixed_no_keep_order_glued", scope: "public")
        mixed_no_keep_order_glued << Expression.new("@{titi} @{tata} @{tutu}", glued: true, aliases: {titi: titi, tata: tata, tutu: tutu}, solution: {titi: "titi", tata: "tata", tutu: "tutu"})

        mixed_keep_order_glued = package.new_interpretation("mixed_keep_order_glued", scope: "public")
        mixed_keep_order_glued << Expression.new("@{toto} @{tata} @{tutu}", keep_order: true, glued: true, aliases: {toto: toto, tata: tata, tutu: tutu}, solution: {toto: "toto", tata: "tata", tutu: "tutu"})

        package
      end

      # Tests

      def test_no_keep_order_no_glued
        expected = { interpretation: "mixed_no_keep_order", solution: {
          "titi" => "titi",
          "toto" => "toto",
          "tutu" => "tutu"
        } }
        check_interpret("titi toto tutu",     expected)
        check_interpret("toto titi tutu",     expected)
      end

      def test_keep_order_no_glued
        expected = { interpretation: "mixed_keep_order", solution: {
          "titi" => "titi",
          "tata" => "tata",
          "toto" => "toto"
        } }
        check_interpret("titi tata toto",     expected)
        exception = assert_raises Minitest::Assertion do
          check_interpret("tata titi toto",     expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")
      end

      def test_no_keep_order_glued
        expected = { interpretation: "mixed_no_keep_order_glued", solution: {
          "titi" => "titi",
          "tata" => "tata",
          "tutu" => "tutu"
        } }
        check_interpret("titi tata tutu",     expected)
        check_interpret("titi tutu tata",     expected)
      end

      def test_keep_order_glued
        expected = { interpretation: "mixed_keep_order_glued", solution: {
          "toto" => "toto",
          "tata" => "tata",
          "tutu" => "tutu"
        } }
        check_interpret("toto tata tutu",     expected)
        exception = assert_raises Minitest::Assertion do
          check_interpret("toto tutu tata",     expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")
      end

    end
  end

end
