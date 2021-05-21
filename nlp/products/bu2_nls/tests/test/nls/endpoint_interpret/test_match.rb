# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNlpMatch < NlsTestCommon

      def create_aller_de_a
        aller_de_a = Package.new("aller-de-a")

        i_new_york = Interpretation.new("new-york").new_textual(["New York", "NYC"])
        aller_de_a << i_new_york

        i_barca = Interpretation.new("barcelona").new_textual(["Barcelona", "Barca"])
        aller_de_a << i_barca

        i_town = Interpretation.new("town")
        i_town.new_expression("@{new-york}", {aliases: {'new-york'  => i_new_york}})
        i_town.new_expression("@{barcelona}", {aliases: { barcelona: i_barca }})
        aller_de_a << i_town

        i_prep_from = Interpretation.new("prep_from").new_textual(["from"])
        aller_de_a << i_prep_from

        i_prep_to = Interpretation.new("prep_to").new_textual(["to"])
        aller_de_a << i_prep_to

        prep_from_town_hash = { 'prep_from' => i_prep_from, town: i_town }
        i_prep_from_town = Interpretation.new("prep_from_town").new_expression("@{prep_from} @{town}", {aliases: prep_from_town_hash})
        aller_de_a << i_prep_from_town

        prep_to_town_hash = { 'prep_to'  => i_prep_to, town: i_town }
        i_prep_to_town = Interpretation.new("prep_to_town").new_expression("@{prep_to} @{town}", {aliases: prep_to_town_hash})
        aller_de_a << i_prep_to_town

        i_go = Interpretation.new("go").new_textual(["go"])
        aller_de_a << i_go

        go_from_to_hash = { go: i_go, from: i_prep_from_town, to: i_prep_to_town }
        i_go_from_to = Interpretation.new("go-from-to").new_expression("@{go} @{from} @{to}", {aliases: go_from_to_hash})
        aller_de_a << i_go_from_to

        i_want = Interpretation.new("want").new_textual(["I want to"])
        aller_de_a << i_want

        want_go_from_to_hash = { want: i_want, 'go-from-to'  => i_go_from_to }
        i_want_go_from_to = Interpretation.new("want-go-from-to").new_expression("@{want} @{go-from-to}", {aliases: want_go_from_to_hash})
        aller_de_a << i_want_go_from_to

        aller_de_a
      end

      def create_aller_de_a_any
        aller_de_a = Package.new("aller-de-a")

        i_prep_from = Interpretation.new("prep_from").new_textual(["from"])
        aller_de_a << i_prep_from

        i_prep_to = Interpretation.new("prep_to").new_textual(["to"])
        aller_de_a << i_prep_to

        prep_from_town_hash = { 'prep_from'  => i_prep_from, town: Alias.any }
        i_prep_from_town = Interpretation.new("prep_from_town").new_expression("@{prep_from} @{town}", {aliases: prep_from_town_hash})
        aller_de_a << i_prep_from_town

        prep_to_town_hash = { 'prep_to'  => i_prep_to, town: Alias.any }
        i_prep_to_town = Interpretation.new("prep_to_town").new_expression("@{prep_to} @{town}", {aliases: prep_to_town_hash})
        aller_de_a << i_prep_to_town

        i_go = Interpretation.new("go").new_textual(["go"])
        aller_de_a << i_go

        go_from_to_hash = { go: i_go, from: i_prep_from_town, to: i_prep_to_town }
        i_go_from_to = Interpretation.new("go-from-to").new_expression("@{go} @{from} @{to}", {aliases: go_from_to_hash})
        aller_de_a << i_go_from_to

        i_want = Interpretation.new("want").new_textual(["I want to"])
        aller_de_a << i_want

        want_go_from_to_hash = { want: i_want, 'go-from-to'  => i_go_from_to }
        i_want_go_from_to = Interpretation.new("want-go-from-to").new_expression("@{want} @{go-from-to}", {aliases: want_go_from_to_hash})
        aller_de_a << i_want_go_from_to

        aller_de_a
      end

      def create_aller_de_a_any_solutions_js
        aller_de_a = Package.new("aller-de-a")

        i_new_york = Interpretation.new("new-york", solution: { "name" => "New York" }).new_textual(["New York", "NYC"])
        aller_de_a << i_new_york

        i_barca = Interpretation.new("barcelona", solution: { "name" => "Barcelona" } ).new_textual(["Barcelona", "Barca"])
        aller_de_a << i_barca

        i_town = Interpretation.new("town")
        i_town.new_expression("@{new-york}", {aliases: {'new-york' => i_new_york}})
        i_town.new_expression("@{barcelona}", {aliases: {barcelona: i_barca}})
        aller_de_a << i_town

        i_prep_from = Interpretation.new("prep_from").new_textual(["from"])
        aller_de_a << i_prep_from

        i_prep_to = Interpretation.new("prep_to").new_textual(["to"])
        aller_de_a << i_prep_to

        prep_to_town_params = {
          aliases: {'prep_to' => i_prep_to, town: i_town},
          keep_order: Expression.keep_order,
          solution: { to: "`town.name`" }
        }
        i_prep_to_town = Interpretation.new("prep_to_town").new_expression("@{prep_to} @{town}", prep_to_town_params)
        prep_to_town_params = {
          aliases: {'prep_to' => i_prep_to, town: Alias.any},
          keep_order: Expression.keep_order,
          solution: { to: "`town`" }
        }
        i_prep_to_town.new_expression("@{prep_to} @{town}", prep_to_town_params)
        aller_de_a << i_prep_to_town

        prep_from_town_params = {
          aliases: { 'prep_from' => i_prep_from, town: i_town },
          keep_order: Expression.keep_order,
          solution: { from: "`town.name`" }
        }
        i_prep_from_town = Interpretation.new("prep_from_town").new_expression("@{prep_from} @{town}", prep_from_town_params)
        prep_from_town_params = {
          aliases: { 'prep_from' => i_prep_from, town: Alias.any },
          keep_order: Expression.keep_order,
          solution: { from: "`town`" }
        }
        i_prep_from_town.new_expression("@{prep_from} @{town}", prep_from_town_params)
        aller_de_a << i_prep_from_town

        i_go = Interpretation.new("go").new_textual(["go"])
        aller_de_a << i_go

        go_from_to_hash = { go: i_go, from: i_prep_from_town, to: i_prep_to_town }
        i_go_from_to = Interpretation.new("go-from-to").new_expression("@{go} @{from} @{to}", {aliases: go_from_to_hash})
        aller_de_a << i_go_from_to

        i_want = Interpretation.new("want").new_textual(["I want to"])
        aller_de_a << i_want

        want_go_from_to_hash = { want: i_want, 'go-from-to' => i_go_from_to }
        i_want_go_from_to = Interpretation.new("want-go-from-to").new_expression("@{want} @{go-from-to}", {aliases: want_go_from_to_hash})
        aller_de_a << i_want_go_from_to

        aller_de_a
      end

      def create_building_feature
        pg_building_feature = Package.new("pg-building-feature")

        i_sea_view = Interpretation.new("sea-view").new_textual(["sea view", "sea front", "ocean view", "ocean front"])
        pg_building_feature << i_sea_view

        i_swimming_pool = Interpretation.new("swimming-pool").new_textual(["swimming pool", "pool"])
        pg_building_feature << i_swimming_pool

        i_building_feature = Interpretation.new("building-feature")
        i_building_feature.new_expression("@{swimming-pool}", {aliases: {'swimming-pool' => i_swimming_pool}})
        i_building_feature.new_expression("@{sea-view}", {aliases: {'sea-view' => i_sea_view}})
        pg_building_feature << i_building_feature

        i_preposition_building_feature= Interpretation.new("preposition-building-feature").new_textual(["with", "at"])
        pg_building_feature << i_preposition_building_feature

        pg_building_feature_hash = {'preposition-building-feature' => i_preposition_building_feature, 'building-feature' => i_building_feature}
        i_pg_building_feature = Interpretation.new("pg-building-feature")
        i_pg_building_feature.new_expression("@{preposition-building-feature} @{building-feature}", {aliases: pg_building_feature_hash})
        i_pg_building_feature.new_expression("@{building-feature}", {aliases: {'building-feature' => i_building_feature}})
        pg_building_feature << i_pg_building_feature

        i_pg_building_features = Interpretation.new("pg-building-features")
        pg_building_features_hash = {'pg-building-feature' => i_pg_building_feature, 'pg-building-features' => i_pg_building_features}
        i_pg_building_features.new_expression("@{pg-building-feature} @{pg-building-features}", {aliases: pg_building_features_hash})
        i_pg_building_features.new_expression("@{pg-building-feature}", {aliases: {'pg-building-feature' => i_pg_building_feature}})
        pg_building_feature << i_pg_building_features

        pg_building_feature
      end

      def create_go_town_recursive
        go_town = Package.new("go-town")

        i_new_york = Interpretation.new("new-york").new_textual(["New York", "NYC"])
        go_town << i_new_york

        i_barca = Interpretation.new("barcelona").new_textual(["Barcelona", "Barca"])
        go_town << i_barca

        i_paris = Interpretation.new("paris").new_textual(["Paris", "Capital of France"])
        go_town << i_paris

        i_town = Interpretation.new("town")
        i_town.new_expression("@{new-york}", {aliases: {'new-york' => i_new_york}})
        i_town.new_expression("@{barcelona}", {aliases: {barcelona: i_barca}})
        i_town.new_expression("@{paris}", {aliases: {paris: i_paris}})
        go_town << i_town

        i_towns = Interpretation.new("towns")
        i_towns.new_expression("@{town} @{towns}", {aliases: {town: i_town, towns: i_towns}})
        i_towns.new_expression("@{town}", {aliases: {town: i_town}})
        go_town << i_towns

        i_go = Interpretation.new("go").new_textual(["go to"])
        go_town << i_go

        i_go_town = Interpretation.new("go-town").new_expression("@{go} @{towns}", {aliases: {go: i_go, towns: i_towns}})
        go_town << i_go_town

        go_town
      end

      def create_deepness_package
        deepness = Package.new("deepness")

        i_id_like = Interpretation.new("id_like", scope: "private").new_textual("I'd like",{locale: "en", keep_order: true, glue_distance: 0})
        i_id_like.new_textual("Je voudrais",{locale: "fr", keep_order: true, glue_distance: 0})
        deepness << i_id_like

        i_to_go = Interpretation.new("to_go", scope: "private").new_textual("to go",{locale: "en", keep_order: true, glue_distance: 0})
        i_to_go.new_textual("aller",{locale: "fr"})
        deepness << i_to_go

        i_prep_to = Interpretation.new("prep_to", scope: "private").new_textual("to",{locale: "en"})
        i_prep_to.new_textual(["à", "en"],{locale: "fr"})
        deepness << i_prep_to

        i_destination = Interpretation.new("destination", scope: "private").new_textual("Paris",{solution: "Paris"})
        i_destination.new_textual("Valence",{solution: "Valence"}).new_textual("Marseille",{solution: "Marseille"})
        deepness << i_destination

        want_go_hash = {'i_id_like' => i_id_like, 'i_to_go' => i_to_go}
        i_want_go = Interpretation.new("want_go", scope: "private").new_expression("@{i_id_like} @{i_to_go}", {aliases: want_go_hash})
        deepness << i_want_go

        main_hash = {'i_want_go' => i_want_go, 'i_prep_to' => i_prep_to, 'i_destination' => i_destination}
        i_main = Interpretation.new("main").new_expression("@{i_want_go} @{i_prep_to} @{i_destination}", {aliases: main_hash})
        deepness << i_main

        main_hash_2 = {'i_id_like' => i_id_like, 'i_to_go' => i_to_go, 'i_prep_to' => i_prep_to, 'i_destination' => i_destination}
        i_main_2 = Interpretation.new("main2").new_expression("@{i_id_like} @{i_to_go} @{i_prep_to} @{i_destination}", {aliases: main_hash_2})
        deepness << i_main_2

        deepness
      end

      def test_interpret_aller_de_a
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        aller_de_a = create_aller_de_a
        Nls.package_update(aller_de_a)

        # resultat attendu
        expected = { interpretation: "want-go-from-to" }

        # creation et exécutio de la requete
        check_interpret("I want to go from Barcelona to New York", expected)

      end

      def test_interpret_aller_de_a_any
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        aller_de_a = create_aller_de_a_any
        Nls.package_update(aller_de_a)

        #resultat attendu
        expected = { interpretation: "want-go-from-to", solution: { town: ["Tokyo", "Paris"] } }

        # creation et exécution de la requete 1
        check_interpret("I want to go from Tokyo to Paris", expected)

        # creation et exécution de la requete 2
        expected = { interpretation: "want-go-from-to", solution: { town: ["New York", "Barcelona"] } }
        check_interpret("I want to go from New York to Barcelona", expected)

        # creation et exécution de la requete 3
        expected = { interpretation: "want-go-from-to", solution: { town: ["no where", "Paris"] } }
        check_interpret("I want to go from no where to Paris", expected)

      end

      def test_interpret_recursive
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        # creation du feed d'init
        go_town = create_go_town_recursive
        Nls.package_update(go_town)

        expected = { interpretation: "go-town" }

         # creation et exécution de la requete
        check_interpret("go to Paris New York Barcelona", expected)

      end

      def test_interpret_builtin_features
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        # feed building features
        pg_building_feature = create_building_feature
        Nls.package_update(pg_building_feature)

        # feed want hotel
        want_hotel = Package.new("want-hotel")

        i_want = Interpretation.new("want").new_textual(["I want", "I need", "I search", "I'm looking for"])
        want_hotel << i_want

        i_hotel = Interpretation.new("hotel").new_textual(["an hotel"])
        want_hotel << i_hotel

        i_want_hotel = Interpretation.new("want-hotel").new_expression("@{want} @{hotel}", {aliases: {want: i_want, hotel: i_hotel}})
        want_hotel << i_want_hotel

        want_hotel_with_feature_hash = {'want-hotel' => i_want_hotel, 'pg-building-features' => pg_building_feature["pg-building-features"]}
        i_want_hotel_with_feature = Interpretation.new("want-hotel-with-feature")
        i_want_hotel_with_feature.new_expression("@{want-hotel} @{pg-building-features}", {aliases: want_hotel_with_feature_hash}) # , want_hotel_with_feature_hash)
        want_hotel << i_want_hotel_with_feature

        Nls.package_update(want_hotel)

        #resultat attendu
        expected = { interpretation: "want-hotel-with-feature" }

        # creation et exécution de la requete
        check_interpret("I want an hotel with swimming pool with sea view", expected)

      end

      def test_building_features
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        # feed building features
        pg_building_feature = create_building_feature
        Nls.package_update(pg_building_feature)

        # resultat attendu
        expected = { interpretation: "pg-building-features" }

        # creation et exécution de la requete
        check_interpret("with swimming pool with sea view", expected)

      end

      def test_building_features_any
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        # feed building feature_any
        pg_building_feature_any = create_building_feature_any
        Nls.package_update(pg_building_feature_any)

        # resultat attendu
        solution = [ { building_feature: "swimming pool"} , { building_feature: "golf" } , { building_feature: "Sea view" } , { building_feature: "spa" } ]
        expected = { interpretation: "pg-building-features", solution: solution, score: 0.65}

        # creation et exécution de la requete
        check_interpret("with swimming pool with golf with sea view with spa", expected)
      end

      def test_with_solutions_with_js
        Nls.remove_all_packages

        Interpretation.default_locale = "en-GB"

        # feed building feature_any
        aller_de_a = create_aller_de_a_any_solutions_js

        Nls.package_update(aller_de_a)

        expected = { interpretation: "want-go-from-to", solution: { from: "New York", to: "Paris"}, score: 0.76}
        check_interpret("I want to go from New York to Paris", expected)

        expected = {interpretation: "want-go-from-to", solution: { from: "Paris", to: "New York"}, score: 0.76}
        check_interpret("I want to go from Paris to New York", expected)

        expected = { interpretation: "want-go-from-to", solution: { from: "Barcelona", to: "New York"}, score: 0.87}
        check_interpret("I want to go from Barcelona to New York", expected)

        expected = { interpretation: "want-go-from-to", solution: { from: "Perpette Les Oies", to: "Barcelona"}, score: 0.73}
        check_interpret("I want to go from Perpette Les Oies to Barcelona", expected)

        expected = { interpretation: "want-go-from-to", solution: { from: "Barcelona", to: "Perpette Les Oies"}, score: 0.73}
        check_interpret("I want to go from Barcelona to Perpette Les Oies", expected)

        expected = { interpretation: "want-go-from-to", solution: { from: "Paris", to: "Perpette Les Oies"}, score: 0.7}
        check_interpret("I want to go from Paris to Perpette Les Oies", expected)

        expected = { interpretation: "want-go-from-to", solution: { from: "Perpette Les Oies", to: "Paris"}, score: 0.7}
        check_interpret("I want to go from Perpette Les Oies to Paris", expected)

      end

      def test_crossed_packages_funny_query
        Nls.remove_all_packages
        Interpretation.default_locale = "en-GB"

        # feed building feature_any
        aller_de_a = create_aller_de_a_any_solutions_js
        Nls.package_update(aller_de_a)

        # feed building features
        pg_building_feature = create_building_feature
        Nls.package_update(pg_building_feature)

        expected = { interpretation: "want-go-from-to", solution: { from: "New York", to: "Barcelona"}, score: 0.84}
        check_interpret("I want to go from New York to Barcelona with swimming pool", expected)
      end


      def test_string_plus_any

        package = Package.new("test_string_plus_any")
        interpretation = package.new_interpretation("test_string_plus_any")
        interpretation.new_expression("string @{any}",  aliases: { any: Alias.any }, solution: "`any`" )

        Nls.package_update(package)

        check_interpret("string hello", interpretation: "test_string_plus_any", solution: "hello")

      end

    end
  end

end
