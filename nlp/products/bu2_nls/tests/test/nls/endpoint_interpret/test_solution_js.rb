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

        interpretation = Interpretation.new("solution_test")

        interpretation << Expression.new("sol null", solution: "`null`")
        interpretation << Expression.new("sol simple", solution: "`'simple'`")
        interpretation << Expression.new("sol number", solution: "`Math.PI`")
        interpretation << Expression.new("sol date", solution: "`new Date('2017-12-03')`")
        interpretation << Expression.new("sol complex", solution: "`var temp = { 'js_key': 'js_value', 'js_array': [] }; temp`")

        nested_solution =
        {
          "key1" => {
            "key_string" => "`'simple'`",
            "key_null" => "`null`",
            "key_number" => "`Math.PI`"
          }
        }
        interpretation << Expression.new("sol nested", solution: nested_solution)

        interpretation << Expression.new("sol main")
        interpretation.solution = "`'main'`"

        package = Package.new("solution_test_package")
        package << interpretation

        package
      end

      def test_solution_null

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test"])

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol null", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

      def test_solution_simple

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test"], 'simple')

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol simple", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

      def test_solution_number

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test"], 3.1415926535897931)

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol number", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

      def test_solution_date

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test"], "2017-12-03T00:00:00.000Z")

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol date", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

      def test_solution_complex

        # resultat attendu
        expected = Answers.new(@solution_test_package["solution_test"], { 'js_key'  => 'js_value', 'js_array' => [] } )

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol complex", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

      def test_solution_nested

        # resultat attendu
        solution =
        { "key1" =>
          {
            "key_string" => "simple",
            "key_null"  => nil,
            "key_number"  => 3.1415926535897931
          }
        }

        expected = Answers.new(@solution_test_package["solution_test"], solution )

        # creation et exécutio de la requete
        request = json_interpret_body(@solution_test_package, "sol nested", Interpretation.default_locale)
        actual = Nls.interpret(request)
        assert_equal expected.to_h, actual

      end

    end
  end

end
