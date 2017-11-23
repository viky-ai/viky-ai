# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestDates < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        Nls.package_update(fixture_parse("package_number_digits.json"))
        Nls.package_update(fixture_parse("package_number_letters.json"))
        Nls.package_update(fixture_parse("package_number.json"))
        Nls.package_update(fixture_parse("package_date.json"))

      end


      def test_simple_date
        check_interpret("3 décembre 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
#        check_interpret("december the third, two thousand and seventeen", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("december the third, 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
#        check_interpret("december the 3, 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("trois decembre deux mille dix sept", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("3/12/2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
      end

      def test_on_date
        check_interpret("le 3 décembre 2017", interpretation: "single_date", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("on december the third, 2017", interpretation: "single_date", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("on 03/12/2017", interpretation: "single_date", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("le 03/12/2017", interpretation: "single_date", solution: { date: "2017-12-03T00:00:00.000Z" } )
      end

      def test_interval_date
        solution = {
        "date_begin" => "2017-12-03T00:00:00.000Z",
        "date_end" => "2018-01-15T00:00:00.000Z"
    }
        check_interpret("du 3 decembre 2017 au 15 janvier 2018", interpretation: "date_interval", solution: solution )
#        check_interpret("from december the 3, 2017 to january the 15, 2018", interpretation: "date_interval", solution: solution )
        sentence = "du trois decembre deux mille dix sept au quinze janvier deux mille dix huit"
        check_interpret(sentence, interpretation: "date_interval", solution: solution )
        sentence = "from december the third, two thousand and seventeen to january the fifteenth, two thousand and eighteen"
#        check_interpret(sentence, interpretation: "date_interval", solution: solution )
        check_interpret("from 3/12/2017 to 15/01/2018", interpretation: "date_interval", solution: solution )
        check_interpret("du 3/12/2017 au 15/01/2018", interpretation: "date_interval", solution: solution )
      end

      def test_extra
#        check_interpret("december the third, two thousand and seventeen", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("december the third, two thousand and seventeen", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
      end

    end
  end
end
