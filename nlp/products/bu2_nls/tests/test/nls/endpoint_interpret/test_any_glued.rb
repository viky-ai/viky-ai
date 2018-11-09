# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyGlued < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("any_glued")

        debut = package.new_interpretation("debut")
        debut << Expression.new("debut", solution: "debut")

        fin = package.new_interpretation("fin")
        fin << Expression.new("fin", solution: "fin")

        word_any_word = package.new_interpretation("word_any_word", { scope: "public" })
        word_any_word << Expression.new("motdebut @{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", glued: true)

        word_any_alias = package.new_interpretation("word_any_alias", { scope: "public" })
        word_any_alias << Expression.new("motdebut @{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", glued: true)

        alias_any_word = package.new_interpretation("alias_any_word", { scope: "public" })
        alias_any_word << Expression.new("@{debut} @{milieu} motfin", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", glued: true)

        alias_any_alias = package.new_interpretation("alias_any_alias", { scope: "public" })
        alias_any_alias << Expression.new("@{debut} @{milieu} @{fin}", aliases: { debut: debut, milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", glued: true)

        any_word = package.new_interpretation("any_word", { scope: "public" })
        any_word << Expression.new("@{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", glued: true)

        any_alias = package.new_interpretation("any_alias", { scope: "public" })
        any_alias << Expression.new("@{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", glued: true)

        word_any = package.new_interpretation("word_any", { scope: "public" })
        word_any << Expression.new("motdebut @{milieu}", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", glued: true)

        alias_any = package.new_interpretation("alias_any", { scope: "public" })
        alias_any << Expression.new("@{debut} @{milieu}", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", glued: true)

        package
      end


      def test_any_keep_order_glued
        # When the keep_order is FALSE, the three possible any 'avant' 'bla bla' and 'apres' are equally possible
        # we choose the last one by default in the algorithm
        expected = { interpretation: "word_any_word", solution: { milieu: "apres"} }
        check_interpret("avant motdebut bla bla motfin apres", expected)

        expected = { interpretation: "word_any_alias", solution: { milieu: "apres"} }
        check_interpret("avant motdebut bla bla fin apres", expected)

        expected = { interpretation: "alias_any_word", solution: { milieu: "apres"} }
        check_interpret("avant debut bla bla motfin apres", expected)

        expected = { interpretation: "alias_any_alias", solution: { milieu: "apres"} }
        check_interpret("avant debut bla bla fin apres", expected)

        expected = { interpretation: "any_word", solution: { milieu: "apres"} }
        check_interpret("bla bla motfin apres", expected)

        expected = { interpretation: "any_alias", solution: { milieu: "apres"} }
        check_interpret("bla bla fin apres", expected)

        expected = { interpretation: "word_any", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebut bla bla", expected)

        expected = { interpretation: "alias_any", solution: { milieu: "bla bla"} }
        check_interpret("avant debut bla bla", expected)

      end

    end
  end

end
