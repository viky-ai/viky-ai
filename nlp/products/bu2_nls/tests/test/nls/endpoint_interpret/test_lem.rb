# frozen_string_literal: true

require 'test_helper'

module Nls
  module EndpointInterpret
    class TestLem < NlsTestCommon
      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = 'fr'

        lem_package = Package.new('lem')

        travailler = lem_package.new_interpretation('travailler')
        travailler << Expression.new('travailler', locale: 'fr')

        # multi bytes test
        etre = lem_package.new_interpretation('etre')
        etre << Expression.new('être', locale: 'fr')

        avoir = lem_package.new_interpretation('avoir')
        avoir << Expression.new('avoir', locale: 'fr')

        cheval = lem_package.new_interpretation('cheval')
        cheval << Expression.new('cheval', locale: 'fr')

        sol = lem_package.new_interpretation('sol')
        sol << Expression.new('sol', locale: 'fr')

        # form
        chiffonnier = lem_package.new_interpretation('chiffonnieres')
        chiffonnier << Expression.new('chiffonnières', locale: 'fr')

        bonhomme = lem_package.new_interpretation('bonshommes')
        bonhomme << Expression.new('bonshommes', locale: 'fr')

        madame = lem_package.new_interpretation('mesdames')
        madame << Expression.new('mesdames', locale: 'fr')

        # no locale fr
        bol = lem_package.new_interpretation('bol')
        bol << Expression.new('bol')

        # en verbs
        vdo = lem_package.new_interpretation('do')
        vdo << Expression.new('do', locale: 'en')

        be = lem_package.new_interpretation('be')
        be << Expression.new('be', locale: 'en')

        # en root
        ladies = lem_package.new_interpretation('ladies')
        ladies << Expression.new('ladies', locale: 'en')

        tooth = lem_package.new_interpretation('tooth')
        tooth << Expression.new('tooth', locale: 'en')

        knives = lem_package.new_interpretation('knives')
        knives << Expression.new('knives', locale: 'en')

        Nls.package_update(lem_package)
      end

      def test_lem_fr_root
        check_interpret('travailler', interpretation: 'travailler', score: 1.0)
        check_interpret('travaillerions', interpretation: 'travailler', score: 0.99)
        check_interpret('chevaux', interpretation: 'cheval', score: 0.99)
        check_interpret('sols', interpretation: 'sol', score: 0.99)

        check_interpret('sommes', interpretation: 'etre', score: 0.99)
        check_interpret('fûmes', interpretation: 'etre', score: 0.99)

        check_interpret('ont', interpretation: 'avoir', score: 0.99)
        check_interpret('eurent', interpretation: 'avoir', score: 0.99)

        exception = assert_raises Minitest::Assertion do
          check_interpret('bols', interpretation: 'bol')
        end
        assert exception.message.include?('Actual answer did not match on any interpretation')
      end

      def test_lem_fr_form
        check_interpret('chiffonnier', interpretation: 'chiffonnieres', score: 0.99)
        check_interpret('bonhomme', interpretation: 'bonshommes', score: 0.99)
        check_interpret('madame', interpretation: 'mesdames', score: 0.99)
      end

      def test_lem_en
        check_interpret('lady',  locale: 'en', interpretation: 'ladies', score: 0.99)
        check_interpret('teeth', locale: 'en', interpretation: 'tooth',  score: 0.99)
        check_interpret('doing', locale: 'en', interpretation: 'do',     score: 0.99)
        check_interpret('been',  locale: 'en', interpretation: 'be',     score: 0.99)
        check_interpret('knife', locale: 'en', interpretation: 'knives', score: 0.99)

      end
    end
  end
end
