# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestLanguage < NlsTestCommon

      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = "*"

        package = Package.new("language")

        determiner = package.new_interpretation("determiner")
        determiner << Expression.new("a", locale: "en")
        determiner << Expression.new("un", locale: "fr")

        animal = package.new_interpretation("animal")
        animal << Expression.new("horse", locale: "en")
        animal << Expression.new("cheval", locale: "fr")

        color = package.new_interpretation("color")
        color << Expression.new("white", locale: "en")
        color << Expression.new("blanc", locale: "fr")

        nominal_group = package.new_interpretation("nominal_group", { scope: "public" })
        nominal_group << Expression.new("@{determiner} @{animal} @{color}", aliases: { determiner: determiner, animal: animal, color: color }, solution: "`{ language: language}`")
        nominal_group << Expression.new("plusieurs @{animal} @{color}", locale: "fr", aliases: { animal: animal, color: color }, solution: "`{ language: language}`")

        Nls.package_update(package)

      end



      def test_language

        check_interpret("un cheval blanc", interpretation: "nominal_group", solution: {language: "fr" })
        check_interpret("un cheval white", interpretation: "nominal_group", solution: {language: "fr" })
        check_interpret("un horse white", interpretation: "nominal_group", solution: {language: "en" })
        check_interpret("plusieurs horse white", interpretation: "nominal_group", solution: {language: "fr" })

      end


    end
  end
end
