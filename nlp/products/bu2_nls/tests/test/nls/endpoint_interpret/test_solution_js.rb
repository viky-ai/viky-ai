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

        Nls.package_update(create_hello_emoji_test_package)
      end

      def create_hello_emoji_test_package
        package = Package.new("hello_emoji_test_package")
        package.default_glued = true
        package.default_keep_order = true

        interpretation = package.new_interpretation("hello_emoji_test")
        interpretation << Expression.new("emoji 👋", solution: '`"👋"`')
        interpretation << Expression.new("emoji 🔥", solution: '`"🔥"`')
        interpretation << Expression.new("emoji 🔥+👋", solution: '`"🔥+👋"`')
        interpretation << Expression.new("emoji array 🔥+👋", solution: '`[ "🔥", "👋" ]`')
        interpretation << Expression.new("emoji abject 🔥+👋", solution: '`{ fire: "🔥", hand: "👋" }`')
        package
      end

      def create_solution_test_package
        package = Package.new("solution_test_package")
        package.default_glued = true
        package.default_keep_order = true

        interpretation = package.new_interpretation("solution_test_js")
        interpretation << Expression.new("sol null", solution: "`null`")
        interpretation << Expression.new("sol simple", solution: "`'simple'`")
        interpretation << Expression.new("sol number", solution: "`Math.PI`")
        interpretation << Expression.new("sol integer", solution: "`3`")
        interpretation << Expression.new("sol date", solution: "`new Date('2017-12-03')`")

        interpretation << Expression.new("sol complex", solution: "` { 'js_key': 'js_value', 'js_array': [] }\n`")

        moment_date_range_sol = <<-eos
          var date1 = moment('2017-12-03T00:00:00+03:00');
          var date2 = moment('2017-12-15T00:00:00+03:00');
          var noel  = moment('2017-12-25', 'YYYY-MM-DD');
          var date_range = moment.range(date1, date2);
          (
            {
              noel: noel,
              date1: date1,
              date2: date2,
              days_diff: date_range.diff('days'),
              months_diff: date_range.diff('months', true),
              duration_json: date_range,
              duration_iso: date_range.toString()
            }
          );
        eos
        interpretation << Expression.new("sol moment-range lib", solution: "`#{moment_date_range_sol}`")
        interpretation << Expression.new("sol moment lib", solution: "`;moment('2017-12-03T00:00:00+03:00')`")

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
        entity      = package.new_interpretation("entity").new_textual("entity").new_textual("dummy entity")
        entity.solution = "entity"

        not_matched = package.new_interpretation("not_matched").new_textual("not_matched")
        no_solution = package.new_interpretation("entity_no_solution").new_textual("entity_no_solution").new_textual("dummy no solution")
        text        = package.new_interpretation("text").new_textual("text")

        match = package.new_interpretation("match")
        match.new_expression("@{entity}",      aliases: { entity: entity }, solution: { name: "`entity`" } )
        match.new_expression("nosol @{number}", aliases: { number: Alias.number })
        match.new_expression("@{number}",      aliases: { number: Alias.number }, solution: { number: "`number`" } )
        match.new_expression("@{not_matched}", aliases: { not_matched: not_matched })
        # @{text} is needed due to a bug in consolidate phase on any
        match.new_expression("@{text} @{entity_any}",  aliases: { text: text, entity_any: Alias.any }, solution: "`entity_any`" )
        match.new_expression("@{no_solution}", aliases: { no_solution: no_solution })
        match.new_expression("a", solution: "`'A'`" )
        match.new_expression("b", solution: "`'B'`" )
        match.new_expression("c", solution: "`'C'`" )

        matches = package.new_interpretation("matches")
        matches.new_expression("@{match}", aliases: { match: match })
        matches.new_expression("@{match} @{matches}", aliases: { match: match, matches: matches }, glued: false)

        solution_test_combine = package.new_interpretation("solution_test_combine")
        solution_test_combine.new_expression("sol combine @{match}", aliases: { match: matches })

        solution_test_2combine = package.new_interpretation("solution_test_2combine")
        solution_test_2combine.new_expression("sol list combine @{combine}", aliases: { combine: solution_test_combine })

        js_isolation_0 = package.new_interpretation("js_isolation_0")
        js_isolation_0 << Expression.new("jsisolationzero", solution: "`'js_isolation_0';`", keep_order: true, glued: false)

        js_isolation_1 = package.new_interpretation("js_isolation_1")
        js_isolation_1 << Expression.new("@{js_isolation_0} jsisolationun", aliases: { js_isolation_0: js_isolation_0 }, solution: "`var undef_variable = 'should not be defined'; 'js_isolation_1';`", keep_order: true, glued: false)
        js_isolation_1 << Expression.new("jsisolationun", solution: "`var undef_variable = 'should not be defined'; 'js_isolation_1';`", keep_order: true, glued: false)
        js_isolation_1 << Expression.new("jsisolationun moment", solution: "`moment = null; 'js_isolation_1';`", keep_order: true, glued: false)
        js_isolation_1 << Expression.new("jsisolationun duktapenlp", solution: "`duktape_nlp = null; 'js_isolation_1';`", keep_order: true, glued: false)

        js_isolation_2 = package.new_interpretation("js_isolation_2")
        js_isolation_2 << Expression.new("@{js_isolation_1} jsisolationdeux"    , aliases: { js_isolation_1: js_isolation_1 }, solution: "`{ js_isolation_0: js_isolation_0, js_isolation_1: js_isolation_1, js_isolation_2: 'js_isolation_2' }`", keep_order: true, glued: false)
        js_isolation_2 << Expression.new("@{js_isolation_1} jsisolationdeux bis", aliases: { js_isolation_1: js_isolation_1 }, solution: "`{ js_isolation_1: js_isolation_1, js_isolation_2: 'js_isolation_2', bug: undef_variable }`", keep_order: true, glued: false)
        js_isolation_2 << Expression.new("jsisolationdeux", solution: "`{ bug: undef_variable }`", keep_order: true, glued: false)

        js_isolation_moment_1 = package.new_interpretation("js_isolation_moment_1")
        js_isolation_moment_1 << Expression.new('js_isolation_moment', solution: "leaks", keep_order: true, glued: false)

        js_isolation_moment = package.new_interpretation('js_isolation_moment')
        js_isolation_moment << Expression.new('@{moment}', aliases: { moment: js_isolation_moment_1, }, solution: "`moment`", keep_order: true, glued: false)

        package
      end

      def test_solution_null
        check_interpret("sol null", interpretation: "solution_test_js", solution: nil)
      end

      def test_solution_simple
        check_interpret("sol simple", interpretation: "solution_test_js", solution: "simple")
      end

      def test_solution_number
        check_interpret("sol number", interpretation: "solution_test_js", solution: 3.141592653589793)
      end

      def test_solution_integer
        check_interpret("sol integer", interpretation: "solution_test_js", solution: 3)
      end

      def test_solution_date
        check_interpret("sol date", interpretation: "solution_test_js", solution: "2017-12-03T00:00:00.000Z")
      end

      def test_solution_moment
        now = "2017-01-01T00:00:00+03:00"
        check_interpret("sol moment lib", interpretation: "solution_test_js", solution: "2017-12-03T00:00:00+03:00", now: now)

        now = "2017-01-01T00:00:00+06:00"
        check_interpret("sol moment lib", interpretation: "solution_test_js", solution: "2017-12-03T03:00:00+06:00", now: now)

        now = "2017-01-01T00:00:00Z"
        check_interpret("sol moment lib", interpretation: "solution_test_js", solution: "2017-12-02T21:00:00Z", now: now)
      end

      def test_solution_moment_range

        now = "2017-01-01T00:00:00+03:00"
        expected_solution = {
          noel:  "2017-12-25T00:00:00+03:00",
          date1: "2017-12-03T00:00:00+03:00",
          date2: "2017-12-15T00:00:00+03:00",
          days_diff: 12,
          months_diff: 0.4,
          duration_json: {
            start: "2017-12-03T00:00:00+03:00",
            :end => "2017-12-15T00:00:00+03:00"
          },
          duration_iso: "2017-12-03T00:00:00+03:00/2017-12-15T00:00:00+03:00"
        }

        check_interpret("sol moment-range lib", interpretation: "solution_test_js", solution: expected_solution, now: now)
      end

      def test_solution_complex

        expected_solution = { 'js_key'  => 'js_value', 'js_array' => [] }
        check_interpret("sol complex", interpretation: "solution_test_js", solution: expected_solution)
      end

      def test_solution_nested
        expected_solution =
        {
          key1:
          {
            key_string: "simple",
            key_null: nil,
            key_number: 3.1415926535897931
          }
        }
        check_interpret("sol nested", interpretation: "solution_test_js", solution: expected_solution)
      end

      def test_solution_combine_unit_sol
        check_interpret("sol combine 1", solution: { match: [ { number: 1 } ] })
        check_interpret("sol combine 2", solution: { match: [ { number: 2 } ] })
      end

      def test_solution_combine_unit_nosol
        check_interpret("sol combine nosol 1", solution: { match: [ 1 ] })
        check_interpret("sol combine nosol 2", solution: { match: [ 2 ] })
      end

      def test_solution_combine_unit_entity
        check_interpret("sol combine entity", solution: { match: [ { name: "entity" } ] } )
        check_interpret("sol combine dummy entity", solution: { match: [ { name: "entity" } ] })
      end

      def test_solution_combine_unit_any
        check_interpret("sol combine text anything", solution: { match: [ "anything" ] })
      end


      def test_solution_combine_complex_sol
        check_interpret("sol combine 1 2 3", solution: { match: [ { number: 1 }, { number: 2 }, { number: 3 } ] })
      end

      def test_solution_combine_complex_nosol
        check_interpret("sol combine nosol 1 nosol 2 nosol 3", solution: { match: [1, 2, 3] })
      end

      def test_solution_combine_complex_entity
        check_interpret("sol combine entity 1 2 3", solution: { match: [ { name: "entity"}, { number: 1 }, { number: 2 }, { number: 3 } ]})
        check_interpret("sol combine entity 1 2 3 entity 4", solution: { match: [ { name: "entity"}, { number: 1 }, { number: 2 }, { number: 3 }, { name: "entity"}, { number: 4 } ]})
        check_interpret("sol combine entity dummy no solution", solution: { match: [ { name: "entity" } ] })
      end

      def test_solution_combine_complex_any
        sol = { match: [ { name: "entity"}, { number: 1 }, { number: 2 }, "everything", { number: 3 }, { name: "entity"}, { number: 4 } ] }
        check_interpret("sol combine entity 1 2 text everything 3 entity 4", solution: sol)
      end

      def test_solution_combine_list_must_be_an_array
        check_interpret("sol combine 1", solution: { match: [ { number: 1 } ] })
        check_interpret("sol combine 1 2", solution: { match: [ { number: 1 }, { number: 2 } ] })
      end

      def test_solution_combine_list_must_be_an_array_in_order
        check_interpret("sol combine a b c", solution: { match: [ 'A', 'B', 'C' ] })
        check_interpret("sol combine b c a", solution: { match: [ 'B', 'C', 'A' ] })
      end

      def test_solution_combine_2_list_in_order
        check_interpret("sol list combine sol combine b b sol combine c a sol combine a b", solution: { match: [ 'B', 'B', 'C', 'A', 'A', 'B' ] })
        check_interpret("sol list combine sol combine c a sol combine b a sol combine c b", solution: { match: [ 'C', 'A', 'B', 'A', 'C', 'B' ] })
      end

      def test_test_hello_emoji
        check_interpret("emoji 👋", solution: "👋")
        check_interpret("emoji 🔥", solution: "🔥")
        check_interpret("emoji 🔥+👋", solution: "🔥+👋")
        check_interpret("emoji array 🔥+👋", solution: [ "🔥", "👋"] )
        check_interpret("emoji object 🔥+👋", solution: { fire: "🔥", hand: "👋" } )
      end

      def test_test_js_isolation_same_request

        exception = assert_raises RestClient::InternalServerError, "'js_isolation_0' must not be undefined" do
          check_interpret("jsisolationzero jsisolationun jsisolationdeux", solution: nil)
        end
        assert exception.message.include?("JavaScript error : ReferenceError: identifier 'js_isolation_0' undefined"), exception.message

        exception = assert_raises RestClient::InternalServerError, "'undef_variable' must not be undefined" do
          check_interpret("jsisolationzero jsisolationun jsisolationdeux bis", solution: nil)
        end
        assert exception.message.include?("JavaScript error : ReferenceError: identifier 'undef_variable' undefined"), exception.message

      end

      def test_test_js_isolation_between_request

        check_interpret("jsisolationun", solution: 'js_isolation_1')
        exception = assert_raises RestClient::InternalServerError, "'undef_variable' must no be undefined" do
          check_interpret("jsisolationdeux", solution: nil )
        end
        assert exception.message.include?("JavaScript error : ReferenceError: identifier 'undef_variable' undefined"), exception.message

      end

      def test_test_js_isolation_core_variables_moment

        # js erase moment variable
        check_interpret("jsisolationun moment", solution: 'js_isolation_1')

        now = "2017-01-01T00:00:00+03:00"
        check_interpret("sol moment lib", interpretation: "solution_test_js", solution: "2017-12-03T00:00:00+03:00", now: now)

      end

      def test_test_js_isolation_core_variables_duktape

        # js erase moment variable
        check_interpret("jsisolationun duktapenlp", solution: 'js_isolation_1')

        now = "2017-01-01T00:00:00+03:00"
        check_interpret("sol moment lib", interpretation: "solution_test_js", solution: "2017-12-03T00:00:00+03:00", now: now)

      end

      def test_solution_moment_isolation
        check_interpret("js_isolation_moment", interpretation: "js_isolation_moment")
        check_interpret("sol date", interpretation: "solution_test_js", solution: "2017-12-03T00:00:00.000Z")
      end

      def test_big_solution

        solution = {}
        (1024*128).times do |i|
          solution["js_key_#{i}"] = "js_value_#{i}"
        end

        package = Package.new('big_sol_test_package')
        package.default_glued = true
        package.default_keep_order = true
        interpretation = package.new_interpretation('big_sol_test')
        interpretation << Expression.new("big sol test", solution: "`#{JSON.pretty_generate(solution)}`")
        Nls.package_update(package)

        File.open("/tmp/temp.json","w") do |f|
          f.write(solution.to_json)
        end

        check_interpret('big sol test', interpretation: 'big_sol_test', solution: solution)
      end

    end
  end

end
