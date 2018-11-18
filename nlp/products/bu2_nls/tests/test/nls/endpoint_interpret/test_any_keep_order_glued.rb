# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestAnyKeepOrderGlued < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new("any_keep_order_glued")

        debut = package.new_interpretation("debut")
        debut << Expression.new("debut", solution: "debut")

        fin = package.new_interpretation("fin")
        fin << Expression.new("fin", solution: "fin")

        word_any_word = package.new_interpretation("word_any_word", { scope: "public" })
        word_any_word << Expression.new("motdebut @{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        word_any_alias = package.new_interpretation("word_any_alias", { scope: "public" })
        word_any_alias << Expression.new("motdebut @{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        alias_any_word = package.new_interpretation("alias_any_word", { scope: "public" })
        alias_any_word << Expression.new("@{debut} @{milieu} motfin", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        alias_any_alias = package.new_interpretation("alias_any_alias", { scope: "public" })
        alias_any_alias << Expression.new("@{debut} @{milieu} @{fin}", aliases: { debut: debut, milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        any_word = package.new_interpretation("any_word", { scope: "public" })
        any_word << Expression.new("@{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        any_alias = package.new_interpretation("any_alias", { scope: "public" })
        any_alias << Expression.new("@{milieu} @{fin}", aliases: { milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        word_any = package.new_interpretation("word_any", { scope: "public" })
        word_any << Expression.new("motdebut @{milieu}", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        alias_any = package.new_interpretation("alias_any", { scope: "public" })
        alias_any << Expression.new("@{debut} @{milieu}", aliases: { debut: debut, milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        package
      end

      def test_any_keep_order_glued

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

      def test_any_keep_order_glued_middle

        Nls.remove_all_packages

        package = Package.new("test_any_keep_order_glued_middle")

        debut = package.new_interpretation("debut", scope: "private")
        debut << Expression.new("debut", solution: "debut")

        debutaaa = package.new_interpretation("debutaaa", scope: "private")
        debutaaa << Expression.new("debutaaa", solution: "debutaaa")

        debutbbb = package.new_interpretation("debutbbb", scope: "private")
        debutbbb << Expression.new("debutbbb", solution: "debutbbb")

        fin = package.new_interpretation("fin", scope: "private" )
        fin << Expression.new("fin", solution: "fin")

        finaaa = package.new_interpretation("finaaa", scope: "private")
        finaaa << Expression.new("finaaa", solution: "finaaa")

        finbbb = package.new_interpretation("finbbb", scope: "private")
        finbbb << Expression.new("finbbb", solution: "finbbb")

        word_word_any_word = package.new_interpretation("word_word_any_word", { scope: "public" })
        word_word_any_word << Expression.new("motdebutaaa motdebutbbb @{milieu} motfin", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        word_any_word_word = package.new_interpretation("word_any_word_word", { scope: "public" })
        word_any_word_word << Expression.new("motdebut @{milieu} motfinaaa motfinbbb", aliases: { milieu: Alias.any}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        alias_alias_any_alias = package.new_interpretation("alias_alias_any_alias", { scope: "public" })
        alias_alias_any_alias << Expression.new("@{debutaaa} @{debutbbb} @{milieu} @{fin}", aliases: { debutaaa: debutaaa, debutbbb: debutbbb, milieu: Alias.any, fin: fin}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        alias_any_alias_alias = package.new_interpretation("alias_any_alias_alias", { scope: "public" })
        alias_any_alias_alias << Expression.new("@{debut} @{milieu} @{finaaa} @{finbbb}", aliases: { debut: debut, milieu: Alias.any, finaaa: finaaa, finbbb: finbbb}, solution: "`{ milieu: milieu}`", keep_order: true, glued: true)

        Nls.package_update(package)

        expected = { interpretation: "word_word_any_word", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebutaaa motdebutbbb bla bla motfin apres", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant motdebutaaa pasbien motdebutbbb bla bla motfin apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant motdebutbbb motdebutaaa bla bla motfin apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "word_any_word_word", solution: { milieu: "bla bla"} }
        check_interpret("avant motdebut bla bla motfinaaa motfinbbb apres", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant motdebut bla bla motfinaaa pasbien motfinbbb apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant motdebut bla bla motfinbbb motfinaaa apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "alias_alias_any_alias", solution: { milieu: "bla bla"} }
        check_interpret("avant debutaaa debutbbb bla bla fin apres", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant debutaaa pasbien debutbbb bla bla fin apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant debutbbb debutaaa bla bla fin apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "alias_any_alias_alias", solution: { milieu: "bla bla"} }
        check_interpret("avant debut bla bla finaaa finbbb apres", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant debut bla bla finaaa pasbien finbbb apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        exception = assert_raises Minitest::Assertion do
          check_interpret("avant debut bla bla finbbb finaaa apres", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

      end

      def test_any_keep_order_glued_ends

        Nls.remove_all_packages

        package = Package.new("test_any_keep_order_glued_ends")

        debut = package.new_interpretation("debut", scope: "private")
        debut << Expression.new("debut", solution: "debut")

        fin = package.new_interpretation("fin", scope: "private" )
        fin << Expression.new("fin", solution: "fin")

        any_word_word = package.new_interpretation("any_word_word", { scope: "public" })
        any_word_word << Expression.new("@{debut} motfinaaa motfinbbb", aliases: { debut: Alias.any}, solution: "`{ debut: debut}`", keep_order: true, glued: true)

        word_word_any = package.new_interpretation("word_word_any", { scope: "public" })
        word_word_any << Expression.new("motdebutaaa motdebutbbb @{fin}", aliases: { fin: Alias.any}, solution: "`{ fin: fin}`", keep_order: true, glued: true)

        any_alias = package.new_interpretation("any_alias", { scope: "public" })
        any_alias << Expression.new("@{debut} @{fin}", aliases: { debut: Alias.any, fin: fin}, solution: "`{ debut: debut}`", keep_order: true, glued: true)

        alias_any = package.new_interpretation("alias_any", { scope: "public" })
        alias_any << Expression.new("@{debut} @{fin}", aliases: { debut: debut, fin: Alias.any}, solution: "`{ fin: fin}`", keep_order: true, glued: true)

        Nls.package_update(package)

        expected = { interpretation: "any_word_word", solution: { debut: "bla bla"} }
        check_interpret("bla bla motfinaaa motfinbbb", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("motfinaaa motfinbbb bla bla", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "word_word_any", solution: { fin: "bla bla"} }
        check_interpret("motdebutaaa motdebutbbb bla bla", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("bla bla motdebutaaa motdebutbbb", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "any_alias", solution: { debut: "bla bla"} }
        check_interpret("bla bla fin", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("fin bla bla", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

        expected = { interpretation: "alias_any", solution: { fin: "bla bla"} }
        check_interpret("debut bla bla", expected)

        exception = assert_raises Minitest::Assertion do
          check_interpret("bla bla debut", expected)
        end
        assert exception.message.include?("Actual answer did not match on any interpretation")

      end

      def test_any_keep_order_topology

        Nls.remove_all_packages

        package = Package.new("test_any_keep_order_topology")

        debut = package.new_interpretation("debut", scope: "private")
        debut << Expression.new("debut", solution: "debut")

        fin = package.new_interpretation("fin", scope: "private" )
        fin << Expression.new("fin", solution: "fin")

        any_alias = package.new_interpretation("any_alias", { scope: "public" })
        any_alias << Expression.new("@{debut} @{fin}", aliases: { debut: Alias.any, fin: fin}, solution: "`{ debut: debut}`", keep_order: true, glued: true)

        alias_any = package.new_interpretation("alias_any", { scope: "public" })
        alias_any << Expression.new("@{debut} @{fin}", aliases: { debut: debut, fin: Alias.any}, solution: "`{ fin: fin}`", keep_order: true, glued: true)

        above_alias_any = package.new_interpretation("above_alias_any", { scope: "public" })
        above_alias_any << Expression.new("@{alias_any} aboveapres", aliases: { alias_any: alias_any }, solution: "`alias_any`", keep_order: true, glued: true)

        above_above_alias_any = package.new_interpretation("above_above_alias_any", { scope: "public" })
        above_above_alias_any << Expression.new("top @{above_alias_any} top", aliases: { above_alias_any: above_alias_any }, solution: "`above_alias_any`", keep_order: true, glued: true)

        above_any_alias = package.new_interpretation("above_any_alias", { scope: "public" })
        above_any_alias << Expression.new("aboveavant @{any_alias}", aliases: { any_alias: any_alias }, solution: "`any_alias`", keep_order: true, glued: true)

        above_above_any_alias = package.new_interpretation("above_above_any_alias", { scope: "public" })
        above_above_any_alias << Expression.new("top @{above_any_alias} top", aliases: { above_any_alias: above_any_alias }, solution: "`above_any_alias`", keep_order: true, glued: true)

        Nls.package_update(package)

        expected = { interpretation: "above_alias_any", solution: { fin: "bla bla"} }
        check_interpret("debut bla bla aboveapres", expected)

        expected = { interpretation: "above_any_alias", solution: { debut: "bla bla"} }
        check_interpret("aboveavant bla bla fin", expected)

        expected = { interpretation: "above_above_alias_any", solution: { fin: "bla bla"} }
        check_interpret("top debut bla bla aboveapres top", expected)

        expected = { interpretation: "above_above_any_alias", solution: { debut: "bla bla"} }
        check_interpret("top aboveavant bla bla fin top", expected)

      end

    end
  end

end
