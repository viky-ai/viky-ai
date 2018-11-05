# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyOverlap < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("package")


        ram_size = package.new_interpretation("ram_size")
        ram_size << Expression.new("@{number} GO RAM", aliases: { number: Alias.number}, solution: "`{ size: number}`")

        int2 = package.new_interpretation("main_interpretation", { scope: "public" })
        int2 << Expression.new("ordinateur @{brand} @{ram_size}", aliases: { brand: Alias.any, ram_size: ram_size }, solution: "`{ brand: brand, size: ram_size.size}`", keep_order: true)

        package
      end

      # Tests

      def test_any_overlap
        expected = {
          solution: {
            "brand" => "dell avec",
            "size" => 12,
          }
        }

        check_interpret("je veux un ordinateur dell avec 12 Go de RAM", expected)
      end

      def test_any_keep_order

        package = Package.new("any_keep_order")

        debut = package.new_interpretation("debut")
        debut << Expression.new("debut", solution: "debut")

        fin = package.new_interpretation("fin")
        fin << Expression.new("fin", solution: "fin")

        word_any_word = package.new_interpretation("word_any_word", { scope: "public" })
        word_any_word << Expression.new("motdebut @{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true)

        word_any_alias = package.new_interpretation("word_any_alias", { scope: "public" })
        word_any_alias << Expression.new("motdebut @{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true)

        alias_any_word = package.new_interpretation("alias_any_word", { scope: "public" })
        alias_any_word << Expression.new("@{debut} @{milieu} motfin", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true)

        alias_any_alias = package.new_interpretation("alias_any_alias", { scope: "public" })
        alias_any_alias << Expression.new("@{debut} @{milieu} @{fin}", aliases: { debut: debut, milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true)

        any_word = package.new_interpretation("any_word", { scope: "public" })
        any_word << Expression.new("@{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true)

        any_alias = package.new_interpretation("any_alias", { scope: "public" })
        any_alias << Expression.new("@{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true)

        word_any = package.new_interpretation("word_any", { scope: "public" })
        word_any << Expression.new("motdebut @{milieu}", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true)

        alias_any = package.new_interpretation("alias_any", { scope: "public" })
        alias_any << Expression.new("@{debut} @{milieu}", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true)

        Nls.remove_all_packages
        Nls.package_update(package)

        expected = { interpretation: "word_any_word", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebut bla bla motfin apres", expected)

        expected = { interpretation: "word_any_alias", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebut bla bla fin apres", expected)

        expected = { interpretation: "alias_any_word", solution: { milieu: "bla bla"} }
        check_interpret("avant debut bla bla motfin apres", expected)

        expected = { interpretation: "alias_any_alias", solution: { milieu: "bla bla"} }
        check_interpret("avant debut bla bla fin apres", expected)

        expected = { interpretation: "any_word", solution: { milieu: "bla bla"} }
        check_interpret("bla bla motfin apres", expected)

        expected = { interpretation: "any_alias", solution: { milieu: "bla bla"} }
        check_interpret("bla bla fin apres", expected)

        expected = { interpretation: "word_any", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebut bla bla", expected)

        expected = { interpretation: "alias_any", solution: { milieu: "bla bla"} }
        check_interpret("avant debut bla bla", expected)

      end

    end
  end

end
