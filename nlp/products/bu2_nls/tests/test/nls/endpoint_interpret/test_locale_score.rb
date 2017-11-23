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

        Nls.package_update(package)

      end



      def test_locale_score_en

        sentence = "with a nice swimming pool with spa for 3 people and sea view"

        check_interpret(sentence, interpretation: "locale", score: 1.0)

      end

      def test_locale_score_fr

        sentence = "with a nice swimming pool with spa for 3 people and sea view"

        Interpretation.default_locale = "fr"

        check_interpret(sentence, interpretation: "locale", score: 0.82)

      end


    end
  end
end
