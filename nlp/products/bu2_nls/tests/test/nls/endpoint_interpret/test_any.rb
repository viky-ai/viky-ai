# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAny < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package())
      end

      def create_package(expression_with_number = false)
        package = Package.new("any_package")

        hello = package.new_interpretation("hello")
        hello << Expression.new("hello @{name}", aliases: { name: Alias.any })
        if expression_with_number
          hello << Expression.new("hello @{number} @{name}", aliases: { number: Alias.number, name: Alias.any })
          hello << Expression.new("hello @{name} @{number}", aliases: { number: Alias.number, name: Alias.any })
        end
        hello << Expression.new("hello")

        no_any = package.new_interpretation("no_any")
        no_any << Expression.new("no_any @{missing}", aliases: { missing: Alias.any })

        package
      end

      # Tests

      def test_any_punctuation_trim

        expected = { interpretation: "hello", solution: "brice" }

        check_interpret("hello brice",        expected)
        check_interpret("hello brice,",       expected)
        check_interpret("hello ,brice",       expected)
        check_interpret("hello ,brice,",      expected)
        check_interpret("hello -brice.",      expected)
        check_interpret("hello :;*brice!+!/", expected)
        check_interpret("hello*brice*",       expected)

      end

      def test_any_punctuation_trim_keep_inside
        check_interpret("hello jean-marc", interpretation: "hello", solution: "jean-marc")
        check_interpret("hello ,jean-marc*", interpretation: "hello", solution: "jean-marc")
        check_interpret("hello ,brice, patrick,", interpretation: "hello", solution: "brice, patrick")
      end

      def test_any_punctuation_only
        check_interpret("hello ,+*", interpretation: "hello",  solution: nil)
      end

      def test_any_missing
        check_interpret("no_any",    interpretation: nil)
        check_interpret("no_any,+*", interpretation: nil)
      end

      def test_any_with_number

        expected = { interpretation: "hello", solution: "123,456.7 brice" }
        check_interpret("hello 123,456.7 brice",   expected)

        expected = { interpretation: "hello", solution: "brice 123,456.7" }
        check_interpret("hello brice 123,456.7",   expected)

      end

      def test_any_with_number_expression

        Nls.remove_all_packages
        Nls.package_update(create_package(true))

        expected = { interpretation: "hello", solution: { number: 123456.7, name: "brice" } }
        check_interpret("hello 123,456.7 brice",   expected)
        check_interpret("hello brice 123,456.7",   expected)

      end

      def test_any_with_order

        package = Package.new("any_with_order")

        voc_in = package.new_interpretation("voc_in", scope: "private")
        voc_in << Expression.new("in")

        voc_ax = package.new_interpretation("voc_ax", scope: "private")
        voc_ax << Expression.new("AA", solution: "AA")
        voc_ax << Expression.new("AB", solution: "AB")
        voc_ax << Expression.new("AC", solution: "AC")

        sub_expression = package.new_interpretation("sub_expression", scope: "private")
        sub_expression << Expression.new("@{voc_in} @{AA}", aliases: {'voc_in' => voc_in, 'AA' => voc_ax}, solution: "`{ a_x: AA }`", keep_order: true, glued: true)
        sub_expression << Expression.new("@{voc_in} @{AA}", aliases: {'voc_in' => voc_in, 'AA' => Alias.any}, solution: "`{ a_x: AA }`", keep_order: true, glued: true)

        element = package.new_interpretation("element", scope: "private")
        element << Expression.new("@{sub_expression}", aliases: {'sub_expression' => sub_expression}, solution: "`sub_expression`")

        list = package.new_interpretation("list", scope: "private")
        list.new_expression("@{element}", aliases: { element: element })
        list.new_expression("@{element} @{list}", aliases: { element: element, list: list }, glued: false)

        super_interpretation = package.new_interpretation("super_interpretation", { scope: "public" })
        opts = { keep_order: true, glued: true, aliases: { list: list }, solution: "`{ a: list.list }`"}
        super_interpretation << Expression.new("@{list}", opts)

        Nls.remove_all_packages
        Nls.package_update(package)

        expected = { interpretation: "super_interpretation", solution: {a: [{ a_x: "AB" }, {a_x: "AB"}, {a_x: "AB"}]} }
        check_interpret("in AB in AB in AB", expected)

        expected = { interpretation: "super_interpretation", solution: {a: [{ a_x: "AB" }, {a_x: "AB"}, {a_x: "ti"}]} }
        check_interpret("in AB in AB in ti", expected)

        expected = { interpretation: "super_interpretation", solution: {a: [{ a_x: "AB" }, {a_x: "ti"}, {a_x: "AB"}]} }
        check_interpret("in AB in ti in AB", expected)
      end


      def test_double_any_in_list

        package = Package.new("double_any_in_list")

        aaa = package.new_interpretation("aaa")
        aaa << Expression.new("aaa", solution: "aaa")

        ccc = package.new_interpretation("ccc")
        ccc << Expression.new("ccc p aaa", solution: "ccc")

        element = package.new_interpretation("element")
        element << Expression.new("p @{ccc}", aliases: { ccc: ccc }, keep_order: true, glued: true)
        element << Expression.new("p @{aaa}", aliases: { aaa: aaa }, keep_order: true, glued: true)
        element << Expression.new("p @{aaa}", aliases: { aaa: Alias.any }, keep_order: true, glued: true)

        element_recursive = package.new_interpretation("element_recursive")
        element_recursive << Expression.new("@{element}", aliases: { element: element }, keep_order: true)
        element_recursive << Expression.new("@{element} @{element_recursive}", aliases: { element: element, element_recursive: element_recursive }, keep_order: true)

        list = package.new_interpretation("list")
        list << Expression.new("@{element}", aliases: { element: element_recursive },  keep_order: true)

        Nls.remove_all_packages
        Nls.package_update(package)


        # no any
        expected = { interpretation: "list", solution: { element: ["aaa", "aaa"] } }
        check_interpret("p aaa p aaa", expected)

        # 1 any
        expected = { interpretation: "list", solution: { element: ["aaa", "bbb"] } }
        check_interpret("p aaa p bbb", expected)

        # 1 any
        expected = { interpretation: "list", solution: { element: ["bbb", "aaa"] } }
        check_interpret("p bbb p aaa", expected)

        # 2 anys
        expected = { interpretation: "list", solution: { element: ["bbb", "bbb"] } }
        check_interpret("p bbb p bbb", expected)

        # perturbation + 1 anys
        expected = { interpretation: "list", solution: { element: ["ccc", "bbb"] } }
        check_interpret("ccc p aaa p bbb", expected)

      end

    end
  end

end
