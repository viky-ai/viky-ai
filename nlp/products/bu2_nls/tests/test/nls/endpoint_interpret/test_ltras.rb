# frozen_string_literal: true

require 'test_helper'

module Nls
  module EndpointInterpret
    class TestLtras < NlsTestCommon
      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = nil

        ltras_package = Package.new('ltras')
        ltras = ltras_package.new_interpretation('ltras')
        ltras << Expression.new('swimming pool with spa for 3 people and sea view')
        Nls.package_update(ltras_package)

        parse_package = Package.new('parse')
        ltras = parse_package.new_interpretation('parse')
        ltras << Expression.new('abc123def,i:jklm456opq;')
        Nls.package_update(parse_package)

        @locale_package = Package.new('locale')
        ltras = @locale_package.new_interpretation('locale_en')
        ltras << Expression.new('complicated', locale: 'en')
        ltras = @locale_package.new_interpretation('locale_fr')
        ltras << Expression.new('complicates', locale: 'fr')
        Nls.package_update(@locale_package)

        @ltras_simple_package = Package.new('ltras_simple')
        ltras_simple = @ltras_simple_package.new_interpretation('ltras_simple')
        ltras_simple << Expression.new('swimming')

        Nls.package_update(@ltras_simple_package)
      end

      def test_ltras_single_words
        sentence = 'with a nice swimmang poool with spa for 3 peoople and sea vieew'
        check_interpret(sentence, interpretation: 'ltras', score: 0.76, spellchecking: :low)
      end

      def test_ltras_simple_locale
        sentence = 'complicate'
        check_interpret(sentence, locale: 'en', packages: [@locale_package], interpretation: 'locale_en', score: 0.8, spellchecking: :high)
        check_interpret(sentence, locale: 'fr', packages: [@locale_package], interpretation: 'locale_fr', score: 0.8, spellchecking: :high)
      end


      def test_ltras_simple_level
        package = @ltras_simple_package

        # inactive
        sentence = 'swimming'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.87, spellchecking: :inactive)

        sentence = 'swiming'
        check_interpret(sentence, packages: [package], interpretation: nil, score: 0.87, spellchecking: :inactive)

        # low
        sentence = 'swiming'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.85, spellchecking: :low)

        sentence = 'swimmming'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.85, spellchecking: :low)

        sentence = 'swimmminng'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.84, spellchecking: :low)

        sentence = 'swimmang'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.80, spellchecking: :low)

        sentence = 'swimmmang'
        check_interpret(sentence, packages: [package], interpretation: nil, spellchecking: :low)

        sentence = 'svimmang'
        check_interpret(sentence, packages: [package], interpretation: nil, spellchecking: :low)

        # medium
        sentence = 'svimmang'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.77, spellchecking: :medium)

        sentence = 'svimmmang'
        check_interpret(sentence, packages: [package], interpretation: nil, spellchecking: :medium)

        # high
        sentence = 'svimmmang'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.76, spellchecking: :high)

        sentence = 'svimmmanng'
        check_interpret(sentence, packages: [package], interpretation: 'ltras_simple', score: 0.76, spellchecking: :high)

        sentence = 'svimmman'
        check_interpret(sentence, packages: [package], interpretation: nil, spellchecking: :high)

      end

      def test_complexe_parsing
        sentence = 'abc 123 def , i : jklm 456 opq ;'
        check_interpret(sentence, interpretation: 'parse', score: 0.87)
      end
    end
  end
end
