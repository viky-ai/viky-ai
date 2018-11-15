# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestGlue < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package())
      end

      def create_package
        package = Package.new("glued_package")

        letters = package.new_interpretation("letters")
        letters << Expression.new("a", solution: "a")
        letters << Expression.new("b", solution: "b")
        letters << Expression.new("@{number}", aliases: { number: Alias.number } )

        threewords = package.new_interpretation("threewords")
        threewords << Expression.new("aaa bbb ccc", solution: "aaa bbb ccc", glued: true)

        dash_expression = package.new_interpretation("dash_expression")
        dash_expression << Expression.new("ddd-eee-fff", solution: "ddd-eee-fff", glued: true, glue_strength: "punctuation")

        glued = package.new_interpretation("glued")
        glued << Expression.new("@{letters1} @{letters2}", aliases: { letters1: letters, letters2: letters }, glued: true)

        package
      end

      # Tests

      def test_glued_ok

        expected = { interpretation: "glued", solution: { letters1: "a", letters2: "b" } }
        check_interpret("a b",     expected)

        expected = { interpretation: "glued", solution: { letters1: "a", letters2: 25 } }
        check_interpret("a 25",     expected)

        expected = { interpretation: "glued", solution: { letters1: "a", letters2: 456 } }
        check_interpret("a 456 toto",     expected)

        expected = { interpretation: "glued", solution: { letters1: "a", letters2: 1 } }
        check_interpret("toto a 1",     expected)

        expected = { interpretation: "glued", solution: { letters1: 2, letters2: "b" } }
        check_interpret("2 b",     expected)

      end

      def test_glued_ko
        expected = { interpretation: "letters", solution: "a" }
        check_interpret("a c b",       expected)
        check_interpret("a,b",       expected)
      end

      def test_glued_no_keep_order
        expected = { interpretation: "threewords", solution: "aaa bbb ccc"}
        check_interpret("aaa bbb ccc" ,expected)
        check_interpret("bbb ccc aaa", expected)
        check_interpret("aaa ccc bbb", expected)
      end

      def test_glued_no_keep_order_glue_expression
        expected = { interpretation: "dash_expression", solution: "ddd-eee-fff"}
        check_interpret("ddd eee fff" ,expected)
        check_interpret("eee fff ddd", expected)
        check_interpret("ddd fff eee", expected)
      end

    end
  end

end
