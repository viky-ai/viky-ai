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

        @now = DateTime.now

      end

      def now
        @now.to_time
      end

      def datetime_utc_formatted(date)
        date_utc = date.utc
        "#{date_utc.year}-#{'%02d'%date_utc.month}-#{'%02d'%date_utc.day}T#{'%02d'%date_utc.hour}:#{'%02d'%date_utc.min}:#{'%02d'%date_utc.sec}.000Z"
      end

      def test_simple_date
        check_interpret("3 décembre 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("december the third, two thousand and seventeen", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("december the third, 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
        check_interpret("december the 3 rd, 2017", interpretation: "date_day_month_year", solution: { date: "2017-12-03T00:00:00.000Z" } )
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
        check_interpret("from december the 3 rd, 2017 to january the 15 th, 2018", interpretation: "date_interval", solution: solution )
        sentence = "du trois decembre deux mille dix sept au quinze janvier deux mille dix huit"
        check_interpret(sentence, interpretation: "date_interval", solution: solution )
        sentence = "from december the third, two thousand and seventeen to january the fifteenth, two thousand and eighteen"
        check_interpret(sentence, interpretation: "date_interval", solution: solution )
        check_interpret("from 3/12/2017 to 15/01/2018", interpretation: "date_interval", solution: solution )
        check_interpret("du 3/12/2017 au 15/01/2018", interpretation: "date_interval", solution: solution )
      end

      def test_complex_date
        date_string = datetime_utc_formatted(now.days_since(3))
        check_interpret("dans 3 jours", interpretation: "latency", solution: {"date" => "#{date_string}"}, now: datetime_utc_formatted(now))
        date_string = datetime_utc_formatted(now.days_since(3))
        check_interpret("dans trois jours", interpretation: "latency", solution: {"date" => "#{date_string}"}, now: datetime_utc_formatted(now))
        date_string = datetime_utc_formatted(now.days_since(3))
        check_interpret("in 3 days", interpretation: "latency", solution: {"date" => "#{date_string}"}, now: datetime_utc_formatted(now))
        date_string = datetime_utc_formatted(now.days_since(3))
        check_interpret("in three days", interpretation: "latency", solution: {"date" => "#{date_string}"}, now: datetime_utc_formatted(now))

        date_begin = datetime_utc_formatted(now.months_since(1).days_since(3))
        date_end = datetime_utc_formatted(now.months_since(1).days_since(10))
        check_interpret("pour 1 semaine dans 1 mois et 3 jours", interpretation: "duration_with_latency", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now.months_since(1).days_since(3))
        date_end = datetime_utc_formatted(now.months_since(1).days_since(10))
        check_interpret("pour une semaine dans un mois et trois jours", interpretation: "duration_with_latency", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now.months_since(1).days_since(3))
        date_end = datetime_utc_formatted(now.months_since(1).days_since(10))
        check_interpret("for one week in one month and three days", interpretation: "duration_with_latency", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now.months_since(1).days_since(3))
        date_end = datetime_utc_formatted(now.months_since(1).days_since(10))
        check_interpret("for 1 week in 1 month and 3 days", interpretation: "duration_with_latency", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))

        date_begin = datetime_utc_formatted(now)
        date_end = datetime_utc_formatted(now.days_since(7))
        check_interpret("pour une semaine", interpretation: "period", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now)
        date_end = datetime_utc_formatted(now.days_since(7))
        check_interpret("pour 1 semaine", interpretation: "period", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now)
        date_end = datetime_utc_formatted(now.days_since(7))
        check_interpret("for one week", interpretation: "period", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
        date_begin = datetime_utc_formatted(now)
        date_end = datetime_utc_formatted(now.days_since(7))
        check_interpret("for 1 week", interpretation: "period", solution: {
          "date_begin" => "#{date_begin}",
          "date_end" => "#{date_end}"
        }, now: datetime_utc_formatted(now))
      end

#      def test_extra
#      end

    end
  end
end
