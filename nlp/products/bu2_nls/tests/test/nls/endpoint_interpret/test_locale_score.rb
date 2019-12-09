# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLocaleScore < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "en"

        package = Package.new("locale")
        locale = package.new_interpretation("locale")
        locale << Expression.new("with a nice swimming pool with spa for 3 people and sea view", locale: "en")
        locale << Expression.new("avec piscine avec spa pour 3 personnes et vue sur la mer", locale: "fr")

        Nls.package_update(package)

      end



      def test_locale_score_en

        sentence = "with a nice swimming pool with spa for 3 people and sea view"

        check_interpret(sentence, locale: "en", interpretation: "locale", score: 1.0)
        check_interpret(sentence, locale: "en;q=0.2", interpretation: "locale", score: 0.86)

      end

      def test_locale_score_fr

        sentence = "avec piscine avec spa pour 3 personnes et vue sur la mer"

        check_interpret(sentence, locale: "fr", interpretation: "locale", score: 1.0)
        check_interpret(sentence, locale: "fr;q=0.2", interpretation: "locale", score: 0.86)


      end


    end
  end
end
