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

      def create_package_wrong_1
        package = Package.new("wrong_package_1")

        value1 = package.new_interpretation("value1", scope: "private")
        value1 << Expression.new("value1", solution: "value1", glue_distance: 0, keep_order: true)

        value2 = package.new_interpretation("value2", scope: "private")
        value2 << Expression.new("value2", solution: "value2", glue_distance: 0, keep_order: true)

        int1 = package.new_interpretation("test", scope: "public")
        int1 << Expression.new("@{value1} @{value2}", aliases: { value1: value1, value2: value2}, solution: "`{ value1: value1, value2: value2}`", glue_distance: 0, keep_order: true)
        #        int1 << Expression.new("@{value1} @{value2}", aliases: { value1: value1, value2: value2}, glue_distance: 0,
# keep_order: true)

        package
      end

      def create_package_wrong_2
        package2 = Package.new("wrong_package_2")

        int1 = package2.new_interpretation("test", scope: "public")
        int1 << Expression.new("test", solution: "`{ value1: value1, value2: value2}`", glue_distance: 0, keep_order: true)

        package2
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
        sub_expression << Expression.new("@{voc_in} @{AA}", aliases: {'voc_in' => voc_in, 'AA' => voc_ax}, solution: "`{ a_x: AA }`", keep_order: true, glue_distance: 0)
        sub_expression << Expression.new("@{voc_in} @{AA}", aliases: {'voc_in' => voc_in, 'AA' => Alias.any}, solution: "`{ a_x: AA }`", keep_order: true, glue_distance: 0)

        element = package.new_interpretation("element", scope: "private")
        element << Expression.new("@{sub_expression}", aliases: {'sub_expression' => sub_expression}, solution: "`sub_expression`")

        list = package.new_interpretation("list", scope: "private")
        list.new_expression("@{element}", aliases: { element: element })
        list.new_expression("@{element} @{list}", aliases: { element: element, list: list })

        super_interpretation = package.new_interpretation("super_interpretation", { scope: "public" })
        opts = { keep_order: true, glue_distance: 0, aliases: { list: list }, solution: "`{ a: list.list }`"}
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
        ccc << Expression.new("ccc ddd hotel", solution: "ccc")

        ddd = package.new_interpretation("ddd")
        ddd << Expression.new("ddd", solution: "ddd")

        element = package.new_interpretation("element")
        # Avec les super lists, on ne crée que la liste complète donc la perturbation va pertuber,
        # mais sur un cas limite, où il est nécessaire d'avoir toutes les combinatoires de listes
        # Il serait possible d'indiquer de ne pas créer une super list sur une liste récursive
        # pour garder toutes la combinatoire, mais ce ne serait que sur des cas très spécifiques.
        #element << Expression.new("@{ddd}", aliases: { ddd: ddd }, keep_order: true, glue_distance: 0)
        element << Expression.new("p @{aaa}", aliases: { aaa: aaa }, keep_order: true, glue_distance: 0)
        element << Expression.new("p @{aaa}", aliases: { aaa: Alias.any }, keep_order: true, glue_distance: 0)

        element_recursive = package.new_interpretation("element_recursive")
        element_recursive << Expression.new("@{element}", aliases: { element: element }, keep_order: true)
        element_recursive << Expression.new("@{element} @{element_recursive}", aliases: { element: element, element_recursive: element_recursive }, keep_order: true)

        list = package.new_interpretation("list")
        list << Expression.new("@{element}", aliases: { element: element_recursive },  keep_order: true)

        ccc_list = package.new_interpretation("ccc_list")
        ccc_list << Expression.new("@{ccc} @{list}", aliases: { ccc: ccc, list: list })


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
        expected = { interpretation: "ccc_list", solution: { ccc: "ccc", element: ["bbb"] } }
        check_interpret("ccc ddd hotel p bbb", expected)

      end

      def test_email_any

        package = Package.new("test_email_any")

        arobase = package.new_interpretation("arobase")
        arobase << Expression.new("@", solution: "@")

        domaineany = package.new_interpretation("domaineany")
        domaineany << Expression.new("domaineany", solution: "domaineany")

        registres_internet = package.new_interpretation("registres_internet")
        registres_internet << Expression.new(".com", solution: ".com")

        email = package.new_interpretation("email", scope: "public")
        email << Expression.new("@{arobase} @{domaineany} @{registres_internet}", aliases: {'arobase' => arobase, 'domaineany' => domaineany, 'registres_internet' => registres_internet}, solution: "`{ domaineany: domaineany }`", keep_order: true)
        email << Expression.new("@{arobase} @{domaineany} @{registres_internet}", aliases: {'arobase' => arobase, 'domaineany' => Alias.any, 'registres_internet' => registres_internet}, solution: "`{ domaineany: domaineany }`", keep_order: true)

        Nls.remove_all_packages
        Nls.package_update(package)

        expected = { interpretation: "email", solution: { domaineany: "soprasteria" } }
        check_interpret("blabla@soprasteria.com", expected)

        expected = { interpretation: "email", solution: { domaineany: "sopra steria" } }
        check_interpret("blabla@sopra steria.com", expected)

        expected = { interpretation: "email", solution: { domaineany: "soprasteria" } }
        check_interpret("blabla@soprasteria .com", expected)

        expected = { interpretation: "email", solution: { domaineany: "s op ra steria" } }
        check_interpret("blabla@ s op ra steria.com", expected)

        expected = { interpretation: "email", solution: { domaineany: "p e r t i m m" } }
        check_interpret("blabla@p e r t i m m.com", expected)

        expected = { interpretation: "email", solution: { domaineany: "p e r t i m m" } }
        check_interpret("blabla@p e r t i m m .com", expected)

        expected = { interpretation: "email", solution: { domaineany: "p e r t i m m" } }
        check_interpret("blabla@ p e r t i m m.com", expected)

        expected = { interpretation: "email", solution: { domaineany: "p e r t i m m" } }
        check_interpret("blabla@ p e r t i m m .com", expected)

      end

    end
  end

end
