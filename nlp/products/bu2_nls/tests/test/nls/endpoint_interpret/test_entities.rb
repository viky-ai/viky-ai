# frozen_string_literal: true

require 'test_helper'

module Nls
  module EndpointInterpret
    class TestEntities < NlsTestCommon
      def setup
        super

        Nls.remove_all_packages

        Interpretation.default_locale = 'en'

        Nls.package_update(create_package)
      end

      def create_package
        package = Package.new('test_entities')

        # we create 500 entities (and a few more) as it is the limit for entities to be created
        car = package.new_interpretation('entities', scope: 'public')

        opts = { keep_order: true, glued: true, glue_strength: 'punctuation' }

        car << Expression.new('maison de campagne', opts, solution: 'maison de campagne')
        car << Expression.new('chemin du petit', opts, solution: 'chemin du petit')
        car << Expression.new('chemin du petit bois', opts, solution: 'chemin du petit bois')
        car << Expression.new('rivière de la belle etoile', opts, solution: 'rivière de la belle etoile')

        dummy_entity1 = 'aaa'
        dummy_entity2 = 'bbb'
        dummy_entity3 = 'ccc'
        501.times do
          dummy_entity1 = dummy_entity1.next
          dummy_entity2 = dummy_entity2.next
          dummy_entity3 = dummy_entity3.next
          dummy_entity = "#{dummy_entity1} #{dummy_entity2} #{dummy_entity3}"
          car << Expression.new(dummy_entity, opts, solution: dummy_entity)
        end

        package
      end

      def test_entities
        expected = { interpretation: nil }
        check_interpret('toto', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne' }
        check_interpret('before maison de campagne after', expected)

        expected = { interpretation: 'entities', solution: 'chemin du petit' }
        check_interpret('before chemin du petit after', expected)

        expected = { interpretation: 'entities', solution: 'chemin du petit bois' }
        check_interpret('before chemin du petit bois after', expected)

        expected = { interpretation: 'entities', solution: 'rivière de la belle etoile' }
        check_interpret('before rivière de la belle etoile after', expected)

        expected = { interpretation: 'entities', solution: 'aab bbc ccd' }
        check_interpret('before aab bbc ccd after', expected)
      end

      def test_entities_lemmatisation
        expected = { interpretation: 'entities', solution: 'maison de campagne' }
        check_interpret('before maisons de campagne after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne' }
        check_interpret('before maison de campagnes after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne' }
        check_interpret('before maisons de campagnes after', expected)

        expected = { interpretation: 'entities', solution: 'rivière de la belle etoile' }
        check_interpret('before rivières de la belle etoile after', expected)
      end

      def test_entities_ltras
        expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.9 }
        check_interpret('before maisson de campagne after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.9 }
        check_interpret('before maisson de cammpagne after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.83 }
        check_interpret('before maison de cempagne after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.83 }
        check_interpret('before maisson de cempagne after', expected)

        # expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.9 }
        # check_interpret('before maisonde campagne after', expected)

        expected = { interpretation: 'entities', solution: 'maison de campagne', score: 0.83 }
        check_interpret('before maison de campane after', expected)
      end
    end
  end
end
