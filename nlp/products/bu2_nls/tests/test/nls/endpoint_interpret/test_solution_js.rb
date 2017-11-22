# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestSolutionJs < NlsTestCommon

      @solution_test_package = nil

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        @solution_test_package = create_solution_test_package()
        Nls.package_update(@solution_test_package)
      end

      def create_solution_test_package
        package = Package.new("solution_test_package")

        interpretation = package.new_interpretation("solution_test_js")
        interpretation << Expression.new("sol null", solution: "`null`")
        interpretation << Expression.new("sol simple", solution: "`'simple'`")
        interpretation << Expression.new("sol number", solution: "`Math.PI`")
        interpretation << Expression.new("sol integer", solution: "`3`")
        interpretation << Expression.new("sol date", solution: "`new Date('2017-12-03')`")
        interpretation << Expression.new("sol complex", solution: "`var temp = { 'js_key': 'js_value', 'js_array': [] }; temp`")

        nested_solution =
        {
          key1: {
            key_string: "`'simple'`",
            key_null: "`null`",
            key_number: "`Math.PI`"
          }
        }
        interpretation << Expression.new("sol nested", solution: nested_solution)

        interpretation << Expression.new("sol main")
        interpretation.solution = "`'main'`"

        # Define recursive and optional match
        entity      = package.new_interpretation("entity").new_textual("entity").new_textual("dummy entity", keep_order: true)
        entity.solution = "entity"

        not_matched = package.new_interpretation("not_matched").new_textual("not_matched")
        no_solution = package.new_interpretation("entity_no_solution").new_textual("entity_no_solution").new_textual("dummy no solution", keep_order: true)
        text        = package.new_interpretation("text").new_textual("text")

        match = package.new_interpretation("match")
        match.new_expression("@{entity}",      aliases: { entity: entity }, solution: { name: "`entity`" } )
        match.new_expression("nosol @{number}", aliases: { number: Alias.digit })
        match.new_expression("@{number}",      aliases: { number: Alias.digit }, solution: { number: "`number`" } )
        match.new_expression("@{not_matched}", aliases: { not_matched: not_matched })
        # @{text} is needed due to a bug in consolidate phase on any
        match.new_expression("@{text} @{entity_any}",  aliases: { text: text, entity_any: Alias.any }, solution: "`entity_any`" )
        match.new_expression("@{no_solution}", aliases: { no_solution: no_solution })

        matches = package.new_interpretation("matches")
        matches.new_expression("@{match}", aliases: { match: match })
        matches.new_expression("@{match} @{matches}", aliases: { match: match, matches: matches }, keep_order: true)

        solution_test_combine = package.new_interpretation("solution_test_combine")
        solution_test_combine.new_expression("sol combine @{matches}", aliases: { matches: matches })

        package
      end

      def test_solution_null

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"])

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol null", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_simple

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"], 'simple')

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol simple", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_number

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"], 3.1415926535897931)

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol number", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_integer

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"], 3)

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol integer", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_date

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"], "2017-12-03T00:00:00.000Z")

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol date", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_complex

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test_js"], { 'js_key'  => 'js_value', 'js_array' => [] } )

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol complex", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_nested

        # resultat attendu
        solution =
        {
          key1:
          {
            key_string: "simple",
            key_null: nil,
            key_number: 3.1415926535897931
          }
        }

        expected = Answers.new(@solution_test_package["solution_test_js"], solution )

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol nested", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_json expected.to_h, actual

      end

      def test_solution_combine_unit

        # expected interpretation base
        expected = Answers.new(@solution_test_package["solution_test_combine"])

        # combine number 1
        request = json_interpret_body(@solution_test_package, "sol combine 1", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { number: 1 }
        assert_json expected.to_h, actual

        # combine number 2
        request = json_interpret_body(@solution_test_package, "sol combine 2", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { number: 2 }
        assert_json expected.to_h, actual

        # combine number 1
        request = json_interpret_body(@solution_test_package, "sol combine nosol 1", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = 1
        assert_json expected.to_h, actual

        # combine number 2
        request = json_interpret_body(@solution_test_package, "sol combine nosol 2", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = 2
        assert_json expected.to_h, actual

        # combine entity
        request = json_interpret_body(@solution_test_package, "sol combine entity", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { name: "entity" }
        assert_json expected.to_h, actual

        # combine dummy entity
        request = json_interpret_body(@solution_test_package, "sol combine dummy entity", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { name: "entity" }
        assert_json expected.to_h, actual

        # combine any
        request = json_interpret_body(@solution_test_package, "sol combine text anything", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.score = 0.91
        expected.first.solution = "anything"
        assert_json expected.to_h, actual

      end


      def test_solution_combine_complex

        # expected interpretation base
        expected = Answers.new(@solution_test_package["solution_test_combine"])

        request = json_interpret_body(@solution_test_package, "sol combine 1 2 3", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { number: [1, 2, 3]}
        assert_json expected.to_h, actual

        request = json_interpret_body(@solution_test_package, "sol combine nosol 1 nosl 2 nosol 3", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { match: [1, 2], matches: 3}
        assert_json expected.to_h, actual

        request = json_interpret_body(@solution_test_package, "sol combine entity 1 2 3", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { number: [1, 2, 3], name: "entity"}
        assert_json expected.to_h, actual

        request = json_interpret_body(@solution_test_package, "sol combine entity 1 2 3 entity 4", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { number: [1, 2, 3, 4], name: [ "entity", "entity" ] }
        assert_json expected.to_h, actual

        request = json_interpret_body(@solution_test_package, "sol combine entity dummy no solution", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.solution = { name: "entity" }
        assert_json expected.to_h, actual

        request = json_interpret_body(@solution_test_package, "sol combine entity 1 2 text everything 3 entity 4", Interpretation.default_locale)
        actual = Nls.interpret(request)
        expected.first.score = 0.94
        expected.first.solution = { number: [1, 2, 3, 4], name: [ "entity", "entity" ], match: "everything" }
        assert_json expected.to_h, actual

      end

    end
  end

end
