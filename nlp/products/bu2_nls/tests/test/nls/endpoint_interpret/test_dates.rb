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

        Nls.package_update(fixture_parse("package_number_digits.json"))
        Nls.package_update(fixture_parse("package_number_letters.json"))
        Nls.package_update(fixture_parse("package_number.json"))
        Nls.package_update(fixture_parse("package_date.json"))
        Nls.package_update(fixture_parse("package_complex_date.json"))
        Nls.package_update(fixture_parse("package_special_dates.json"))
        Nls.package_update(fixture_parse("package_overall_date.json"))
        Nls.package_update(fixture_parse("package_date_grammar.json"))
        Nls.package_update(fixture_parse("package_date_vocabulary.json"))
      end

      def test_simple_date
        expected = "2017-12-03T00:00:00Z"
        check_interpret("december the third, two thousand and seventeen", locale: "en", interpretation: "date", solution: { date: expected } )
        check_interpret("december the third, 2017", interpretation: "date", locale: "en", solution: { date: expected } )
        check_interpret("december the 3 rd, 2017", interpretation: "date", locale: "en", solution: { date: expected } )
        check_interpret("3 décembre 2017", interpretation: "date", locale: "fr", solution: { date: expected } )
        check_interpret("trois decembre deux mille dix sept", locale: "fr", interpretation: "date", solution: { date: expected } )
        check_interpret("3/12/2017", interpretation: "date", locale: "fr", solution: { date: expected } )
      end

      def test_on_date
        expected = "2017-12-03T00:00:00Z"
        check_interpret("on december the third, 2017", locale: "en", interpretation: "date", solution: {date: expected})
        check_interpret("on 03/12/2017", locale: "en", interpretation: "date", solution: {date: expected})
        check_interpret("le 3 décembre 2017", locale: "fr", interpretation: "date", solution: {date: expected})
        check_interpret("le 03/12/2017", locale: "fr", interpretation: "date", solution: {date: expected})
      end

      def test_interval_date
        solution = {"date_range" =>
          {"start"=>"2017-12-03T00:00:00Z", "end"=>"2018-01-15T00:00:00Z"}
        }
        check_interpret("from december the 3 rd, 2017 to january the 15 th, 2018", locale: "en", interpretation: "date_range", solution: solution )
        sentence = "from december the third, two thousand and seventeen to january the fifteenth, two thousand and eighteen"
        check_interpret(sentence, locale: "en", interpretation: "date_range", solution: solution )
        check_interpret("from 3/12/2017 to 15/01/2018", locale: "en", interpretation: "date_range", solution: solution )
        sentence = "du trois decembre deux mille dix sept au quinze janvier deux mille dix huit"
        check_interpret(sentence, locale: "fr", interpretation: "date_range", solution: solution )
        check_interpret("du 3 decembre 2017 au 15 janvier 2018", locale: "fr", interpretation: "date_range", solution: solution )
        check_interpret("du 3/12/2017 au 15/01/2018", locale: "fr", interpretation: "date_range", solution: solution )
      end

      def test_complex_date_in_period
        now = "2016-02-28T00:00:00+03:00"

        expected = {"date_range"=>
          {"start"=>"2016-03-02T00:00:00+03:00", "end"=>"2016-03-02T23:59:59+03:00"}
        }
        check_interpret("in 3 days", locale: "en", interpretation: "date_range", solution: expected, now: now)
        check_interpret("in three days", locale: "en", interpretation: "date_range", solution: expected, now: now)

        check_interpret("dans 3 jours", locale: "fr", interpretation: "date_range", solution: expected, now: now)
        check_interpret("dans trois jours", locale: "fr", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-03-13T00:00:00+03:00", "end"=>"2016-03-19T23:59:59+03:00"}
        }
        check_interpret("dans 2 semaines", locale: "fr", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-03-01T00:00:00+03:00", "end"=>"2016-03-31T23:59:59+03:00"}
        }
        check_interpret("dans 1 mois", locale: "fr", interpretation: "date_range", solution: expected, now: now)

        expected = {"date_range"=>
          {"start"=>"2016-06-05T00:00:00+03:00", "end"=>"2016-06-05T23:59:59+03:00"}
        }
        check_interpret("dans 3 mois 1 semaine et 1 jour", locale: "fr", interpretation: "date_range", solution: expected, now: now)
      end

      def test_complex_date_in_period_for_period
        now = "2016-02-28T00:00:00+03:00"

        result = {
          "date_range" =>
            {
              "start" => "2016-03-31T00:00:00+03:00",
              "end"   => "2016-04-07T00:00:00+03:00"
            }
        }
        check_interpret("for one week in one month and three days", locale: "en", interpretation: "date_range", solution: result, now: now)
        check_interpret("for 1 week in 1 month and 3 days", locale: "en", interpretation: "date_range", solution: result, now: now)
        check_interpret("pour 1 semaine dans 1 mois et 3 jours", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("pour une semaine dans un mois et trois jours", locale: "fr", interpretation: "date_range", solution: result, now: now)
      end

      def test_complex_date_for_period
        now = "2016-02-28T00:00:00+03:00"

        solution = { "duration" => { "value" => "P7D", "unit" => "week"} }
        check_interpret("pour une semaine", locale: "fr", interpretation: "duration_overall", solution: solution, now: now)
        check_interpret("pour 1 semaine", locale: "fr", interpretation: "duration_overall", solution: solution, now: now)

        check_interpret("for one week", locale: "en", interpretation: "duration_overall", solution: solution, now: now)
        check_interpret("for 1 week", locale: "en", interpretation: "duration_overall", solution: solution, now: now)

        solution = { "duration" => { "value" => "P3M", "unit" => "month"} }
        check_interpret("for 3 months", locale: "en", interpretation: "duration_overall", solution: solution, now: now)

        solution = { "duration" => { "value" => "P3M2D", "unit" => "day"} }
        check_interpret("for 3 months and 2 days", locale: "en", interpretation: "duration_overall", solution: solution, now: now)

      end

      def test_special_dates
        now = "2016-02-28T00:00:00+03:00"
        expected = {
          "date_range" => {
            "start" => "2016-12-23T00:00:00+03:00",
            "end" => "2016-12-27T00:00:00+03:00"
          }
        }
        check_interpret("pour noel", locale: "fr", interpretation: "date_range", solution: expected, now: now)
        expected = {
          "date_range" => {
            "start" => "2017-02-12T00:00:00+03:00",
            "end" => "2017-02-16T00:00:00+03:00"
          }
        }
        check_interpret("pour la saint valentin", locale: "fr", interpretation: "date_range", solution: expected, now: now)
        check_interpret("for valentine", locale: "en", interpretation: "date_range", solution: expected, now: now)
      end

      def test_week_of_day
        now = "2017-11-28T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2017-12-24T00:00:00+03:00", "end"=>"2017-12-30T23:59:59+03:00"}
        }
        check_interpret("la semaine de noel", locale: "fr", interpretation: "date_range", solution: result, now: now)
      end

      def test_next_weekday
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi prochain", locale: "fr", interpretation: "date", solution: {"date" => "2017-11-30T00:00:00+03:00"}, now: now)
        check_interpret("mardi prochain", locale: "fr", interpretation: "date", solution: {"date" => "2017-12-05T00:00:00+03:00"}, now: now)
        check_interpret("next thursday", locale: "en", interpretation: "date", solution: {"date" => "2017-11-30T00:00:00+03:00"}, now: now)
        check_interpret("next tuesday", locale: "en", interpretation: "date", solution: {"date" => "2017-12-05T00:00:00+03:00"}, now: now)
      end

      def test_last_weekday
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi dernier", locale: "fr", interpretation: "date", solution: {"date" => "2017-11-23T00:00:00+03:00"}, now: now)
        check_interpret("mardi dernier", locale: "fr", interpretation: "date", solution: {"date" => "2017-11-28T00:00:00+03:00"}, now: now)
        check_interpret("last thursday", locale: "en", interpretation: "date", solution: {"date" => "2017-11-23T00:00:00+03:00"}, now: now)
        check_interpret("last tuesday", locale: "en", interpretation: "date", solution: {"date" => "2017-11-28T00:00:00+03:00"}, now: now)
      end

      def test_dayweek_in_n_weeks
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi dans 3 semaines", locale: "fr", interpretation: "date",  solution: {"date" => "2017-12-21T00:00:00+03:00"}, now: now)
        check_interpret("thursday in 3 weeks", locale: "en", interpretation: "date", solution: {"date" => "2017-12-21T00:00:00+03:00"}, now: now)
      end

      def test_dayweek_n_weeks_ago
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("jeudi il y a 3 semaines", locale: "fr", interpretation: "date", solution: {"date" => "2017-11-09T00:00:00+03:00"}, now: now)
        check_interpret("thursday 3 weeks ago", locale: "en", interpretation: "date", solution: {"date" => "2017-11-09T00:00:00+03:00"}, now: now)
      end

      def test_date_of_next_month
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 5 dans 2 mois", locale: "fr", interpretation: "date", solution: {"date" => "2018-01-05T00:00:00+03:00"}, now: now)
        check_interpret("the 12 in 5 months", locale: "en", interpretation: "date", solution: {"date" => "2018-04-12T00:00:00+03:00"}, now: now)
      end

      def test_date_of_last_month
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 21 il y a 3 mois", locale: "fr", interpretation: "date", solution: {"date" => "2017-08-21T00:00:00+03:00"}, now: now)
        check_interpret("the 12 4 months ago", locale: "en", interpretation: "date", solution: {"date" => "2017-07-12T00:00:00+03:00"}, now: now)
      end

      def test_date_in_n_years
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 16 juin dans 3 ans", locale: "fr", interpretation: "date", solution: {"date" => "2020-06-16T00:00:00+03:00"}, now: now)
        check_interpret("april the 25 th in 4 years", locale: "en", interpretation: "date", solution: {"date" => "2021-04-25T00:00:00+03:00"}, now: now)
      end

      def test_date_n_years_ago
        now = "2017-11-29T00:00:00+03:00"
        check_interpret("le 21 mars il y a 3 ans", locale: "fr", interpretation: "date", solution: {"date" => "2014-03-21T00:00:00+03:00"}, now: now)
        check_interpret("april the 12 th 4 years ago", locale: "en", interpretation: "date", solution: {"date" => "2013-04-12T00:00:00+03:00"}, now: now)
      end

      def test_next_special_date
        now = "2016-02-28T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2016-12-23T00:00:00+03:00", "end"=>"2016-12-27T00:00:00+03:00"}
        }
        check_interpret("noel prochain", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next Xmas", locale: "en", interpretation: "date_range", solution: result, now: now)
        result = {
          "date_range" =>
          {"start" => "2017-02-12T00:00:00+03:00", "end"=>"2017-02-16T00:00:00+03:00"}
        }
        check_interpret("la saint valentin prochaine", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next valentine's day", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_next_week_month_year
        now = "2016-02-25T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2016-02-28T00:00:00+03:00", "end" => "2016-03-05T23:59:59+03:00"}
        }

        check_interpret("la semaine prochaine", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next week", locale: "en", interpretation: "date_range", solution: result, now: now)
        result = {
          "date_range" =>
          {"start" => "2016-03-01T00:00:00+03:00", "end" => "2016-03-31T23:59:59+03:00"}
        }
        check_interpret("le mois prochain", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next month", locale: "en", interpretation: "date_range", solution: result, now: now)
        result = {
          "date_range" =>
          {"start" => "2017-01-01T00:00:00+03:00", "end" => "2017-12-31T23:59:59+03:00"}
        }
        check_interpret("l'année prochaine", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next year", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_last_week_month_year
        now = "2016-02-25T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2016-02-14T00:00:00+03:00", "end" => "2016-02-20T23:59:59+03:00"}
        }

        check_interpret("la semaine dernière", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("last week", locale: "en", interpretation: "date_range", solution: result, now: now)
        result = {
          "date_range" =>
          {"start" => "2016-01-01T00:00:00+03:00", "end" => "2016-01-31T23:59:59+03:00"}
        }
        check_interpret("le mois dernier", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("last month", locale: "en", interpretation: "date_range", solution: result, now: now)
        result = {
          "date_range" =>
          {"start" => "2015-01-01T00:00:00+03:00", "end" => "2015-12-31T23:59:59+03:00"}
        }
        check_interpret("l'année dernière", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("last year", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_next_monthname
        now = "2016-02-25T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2016-05-01T00:00:00+03:00", "end" => "2016-05-31T23:59:59+03:00"}
        }
        check_interpret("en mai prochain", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next may", locale: "en", interpretation: "date_range", solution: result, now: now)
        now = "2016-05-15T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2017-05-01T00:00:00+03:00", "end" => "2017-05-31T23:59:59+03:00"}
        }
        check_interpret("en mai prochain", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next may", locale: "en", interpretation: "date_range", solution: result, now: now)
        now = "2016-07-25T00:00:00+03:00"
        check_interpret("en mai prochain", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("next may", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_monthname_year
        now = "2016-02-25T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2025-05-01T00:00:00+03:00", "end" => "2025-05-31T23:59:59+03:00"}
        }
        check_interpret("en mai 2025", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("in may 2025", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_specialdate_year
        now = "2016-02-25T00:00:00+03:00"
        result = {
          "date_range" =>
          {"start" => "2025-12-23T00:00:00+03:00", "end" => "2025-12-27T00:00:00+03:00"}
        }
        check_interpret("noel 2025", locale: "fr", interpretation: "date_range", solution: result, now: now)
        check_interpret("christmas 2025", locale: "en", interpretation: "date_range", solution: result, now: now)
      end

      def test_closedays
        now = "2016-02-28T00:00:00+03:00"
        Interpretation.default_locale = "fr-FR"
        check_interpret("avant hier", locale: "fr", interpretation: "date", solution: {"date"=> "2016-02-26T00:00:00+03:00" }, now: now)
        check_interpret("hier", locale: "fr", interpretation: "date", solution: {"date"=> "2016-02-27T00:00:00+03:00" }, now: now)
        check_interpret("aujourd'hui", locale: "fr", interpretation: "date", solution: {"date"=> "2016-02-28T00:00:00+03:00" }, now: now)
        check_interpret("demain", locale: "fr", interpretation: "date", solution: {"date"=> "2016-02-29T00:00:00+03:00" }, now: now)
        check_interpret("apres demain", locale: "fr", interpretation: "date", solution: {"date"=> "2016-03-01T00:00:00+03:00" }, now: now)
      end




      #      def test_extra
      #      end

    end
  end
end
