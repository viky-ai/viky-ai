# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestDates < NlsTestCommon

      def set_now(date)
        @now = DateTime(date)
      end

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        Nls.package_update(fixture_parse("package_number_digits.json"))
        Nls.package_update(fixture_parse("package_number_letters.json"))
        Nls.package_update(fixture_parse("package_number.json"))
        Nls.package_update(fixture_parse("package_date.json"))
        Nls.package_update(fixture_parse("package_complex_date.json"))
        Nls.package_update(fixture_parse("package_special_dates.json"))
        Nls.package_update(fixture_parse("package_overall_date.json"))
      end

      def test_simple_date
        expected = "2017-12-03T00:00:00Z"
        check_interpret("3 décembre 2017", interpretation: "date", solution: { date: expected } )
        check_interpret("december the third, two thousand and seventeen", interpretation: "date", solution: { date: expected } )
        check_interpret("december the third, 2017", interpretation: "date", solution: { date: expected } )
        check_interpret("december the 3 rd, 2017", interpretation: "date", solution: { date: expected } )
        check_interpret("trois decembre deux mille dix sept", interpretation: "date", solution: { date: expected } )
        check_interpret("3/12/2017", interpretation: "date", solution: { date: expected } )
      end

      def test_on_date
        expected = "2017-12-03T00:00:00Z"

        check_interpret("le 3 décembre 2017", interpretation: "date", solution: {date: expected})
        check_interpret("on december the third, 2017", interpretation: "date", solution: {date: expected})
        check_interpret("on 03/12/2017", interpretation: "date", solution: {date: expected})
        check_interpret("le 03/12/2017", interpretation: "date", solution: {date: expected})
      end

      def test_interval_date
        solution = {"date_range" =>
          {"start"=>"2017-12-03T00:00:00Z", "end"=>"2018-01-15T00:00:00Z"}
        }

        check_interpret("du 3 decembre 2017 au 15 janvier 2018", interpretation: "date_range", solution: solution )
        check_interpret("from december the 3 rd, 2017 to january the 15 th, 2018", interpretation: "date_range", solution: solution )
        sentence = "du trois decembre deux mille dix sept au quinze janvier deux mille dix huit"
        check_interpret(sentence, interpretation: "date_range", solution: solution )
        sentence = "from december the third, two thousand and seventeen to january the fifteenth, two thousand and eighteen"
        check_interpret(sentence, interpretation: "date_range", solution: solution )
        check_interpret("from 3/12/2017 to 15/01/2018", interpretation: "date_range", solution: solution )
        check_interpret("du 3/12/2017 au 15/01/2018", interpretation: "date_range", solution: solution )
      end

      def test_complex_date_in_period
        now = "2016-02-28T00:00:00+03:00"
        expected = {"date_range"=>
          {"start"=>"2016-03-02T00:00:00+03:00", "end"=>"2016-03-02T23:59:59+03:00"}
        }
        check_interpret("dans 3 jours", interpretation: "date_range", solution: expected, now: now)
        check_interpret("dans trois jours", interpretation: "date_range", solution: expected, now: now)
        check_interpret("in 3 days", interpretation: "date_range", solution: expected, now: now)
        check_interpret("in three days", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-03-13T00:00:00+03:00", "end"=>"2016-03-19T23:59:59+03:00"}
        }
        check_interpret("dans 2 semaines", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-03-01T00:00:00+03:00", "end"=>"2016-03-31T23:59:59+03:00"}
        }
        check_interpret("dans 1 mois", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-06-05T00:00:00+03:00", "end"=>"2016-06-05T23:59:59+03:00"}
        }
        check_interpret("dans 3 mois 1 semaine et 1 jour", interpretation: "date_range", solution: expected, now: now)
      end

      def test_complex_date_in_period_for_period
        now = "2016-02-28T00:00:00+03:00"

        result = {
          "date_range" =>
          {"start" => "2016-03-31T00:00:00+03:00", "end"=>"2016-04-07T00:00:00+03:00"}
        }
        check_interpret("pour 1 semaine dans 1 mois et 3 jours", interpretation: "date_range", solution: result, now: now)
        check_interpret("pour une semaine dans un mois et trois jours", interpretation: "date_range", solution: result, now: now)
        check_interpret("for one week in one month and three days", interpretation: "date_range", solution: result, now: now)
        check_interpret("for 1 week in 1 month and 3 days", interpretation: "date_range", solution: result, now: now)
      end

      def test_complex_date_for_period
        now = "2016-02-28T00:00:00+03:00"

        duration = "P7D"
        check_interpret("pour une semaine", interpretation: "duration", solution: {"duration" => duration}, now: now)
        check_interpret("pour 1 semaine", interpretation: "duration", solution: {"duration" => duration}, now: now)
        check_interpret("for one week", interpretation: "duration", solution: {"duration" => duration}, now: now)
        check_interpret("for 1 week", interpretation: "duration", solution: {"duration" => duration}, now: now)

        duration = "P3M"
        check_interpret("for 3 months", interpretation: "duration", solution: {"duration" => duration}, now: now)

        duration = "P3M2D"
        check_interpret("for 3 months and 2 days", interpretation: "duration", solution: {"duration" => duration}, now: now)

      end

      def test_special_dates
        now = "2016-02-28T00:00:00+03:00"
        check_interpret("pour noel", interpretation: "date", solution: {"date" => "2016-12-25T00:00:00+03:00"}, now: now)
        check_interpret("pour la saint valentin", interpretation: "date", solution: {"date" => "2016-02-14T00:00:00+03:00"}, now: now)
        check_interpret("for valentine", interpretation: "date", solution: {"date" => "2016-02-14T00:00:00+03:00"}, now: now)
      end

      def test_week_of_day
        now = "2017-11-28T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2017-12-24T00:00:00+03:00", "end"=>"2017-12-30T23:59:59+03:00"}
        }


        check_interpret("la semaine de noel", interpretation: "date_range", solution: result, now: now)
      end

      def test_next_weekday
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi prochain", interpretation: "date", solution: {"date" => "2017-11-30T00:00:00+03:00"}, now: now)
        check_interpret("next thursday", interpretation: "date", solution: {"date" => "2017-11-30T00:00:00+03:00"}, now: now)
        check_interpret("mardi prochain", interpretation: "date", solution: {"date" => "2017-12-05T00:00:00+03:00"}, now: now)
        check_interpret("next tuesday", interpretation: "date", solution: {"date" => "2017-12-05T00:00:00+03:00"}, now: now)
      end

      def test_last_weekday
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi dernier", interpretation: "date", solution: {"date" => "2017-11-23T00:00:00+03:00"}, now: now)
        check_interpret("last thursday", interpretation: "date", solution: {"date" => "2017-11-23T00:00:00+03:00"}, now: now)
        check_interpret("mardi dernier", interpretation: "date", solution: {"date" => "2017-11-28T00:00:00+03:00"}, now: now)
        check_interpret("last tuesday", interpretation: "date", solution: {"date" => "2017-11-28T00:00:00+03:00"}, now: now)
      end

      def test_dayweek_in_n_weeks
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi dans 3 semaines", interpretation: "date", solution: {"date" => "2017-12-21T00:00:00+03:00"}, now: now)
        check_interpret("thursday in 3 weeks", interpretation: "date", solution: {"date" => "2017-12-21T00:00:00+03:00"}, now: now)
      end

      def test_dayweek_n_weeks_ago
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi il y a 3 semaines", interpretation: "date", solution: {"date" => "2017-11-09T00:00:00+03:00"}, now: now)
        check_interpret("thursday 3 weeks ago", interpretation: "date", solution: {"date" => "2017-11-09T00:00:00+03:00"}, now: now)
      end

      def test_date_of_next_month
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 5 dans 2 mois", interpretation: "date", solution: {"date" => "2018-01-05T00:00:00+03:00"}, now: now)
        check_interpret("the 12 in 5 months", interpretation: "date", solution: {"date" => "2018-04-12T00:00:00+03:00"}, now: now)
      end

      def test_date_of_last_month
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 21 il y a 3 mois", interpretation: "date", solution: {"date" => "2017-08-21T00:00:00+03:00"}, now: now)
        check_interpret("the 12 4 months ago", interpretation: "date", solution: {"date" => "2017-07-12T00:00:00+03:00"}, now: now)
      end

      def test_date_in_n_years
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 16 juin dans 3 ans", interpretation: "date", solution: {"date" => "2020-06-16T00:00:00+03:00"}, now: now)
        check_interpret("april the 25 th in 4 years", interpretation: "date", solution: {"date" => "2021-04-25T00:00:00+03:00"}, now: now)
      end

      def test_date_n_years_ago
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 21 mars il y a 3 ans", interpretation: "date", solution: {"date" => "2014-03-21T00:00:00+03:00"}, now: now)
        check_interpret("april the 12 th 4 years ago", interpretation: "date", solution: {"date" => "2013-04-12T00:00:00+03:00"}, now: now)
      end

      def tset_next_special_date
        now = "2016-02-28T00:00:00+03:00"
        check_interpret("noel prochain", interpretation: "date", solution: {"date" => "2016-12-25T00:00:00+03:00"}, now: now)
        check_interpret("next Xmas", interpretation: "date", solution: {"date" => "2016-12-25T00:00:00+03:00"}, now: now)
        check_interpret("la saint valentin prochaine", interpretation: "date", solution: {"date" => "2017-02-14T00:00:00+03:00"}, now: now)
        check_interpret("next valentine's day", interpretation: "date", solution: {"date" => "2017-02-14T00:00:00+03:00"}, now: now)
      end


      #      def test_extra
      #      end

    end
  end
end
