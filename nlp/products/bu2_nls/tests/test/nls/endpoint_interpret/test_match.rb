# encoding: utf-8

require 'test_helper'

module Nls

  module EndpointInterpret

    class TestNlpMatch < NlsTestCommon

      def test_interpret_request

        Interpretation.default_locale = "en-GB"

        # creation du feed d'init
        aller_de_a = Package.new("aller-de-a")

        i_new_york = Interpretation.new("new-york").new_textual(["New York", "NYC"])
        aller_de_a << i_new_york
        alias_newyork = Alias.new(i_new_york)

        i_barca = Interpretation.new("barcelona").new_textual(["Barcelona", "Barca"])
        aller_de_a << i_barca
        alias_barca = Alias.new(i_barca)

        i_town = Interpretation.new("town").new_expression("@{new-york}", [alias_newyork])
        i_town.new_expression("@{barcelona}", [alias_barca])
        aller_de_a << i_town
        alias_town = Alias.new(i_town)

        i_prep_from = Interpretation.new("prep-from").new_textual(["from"])
        aller_de_a << i_prep_from
        alias_prep_from = Alias.new(i_prep_from)

        i_prep_to = Interpretation.new("prep-to").new_textual(["to"])
        aller_de_a << i_prep_to
        alias_prep_to = Alias.new(i_prep_to)

        prep_from_town_array = [ alias_prep_from, alias_town ]
        i_prep_from_town = Interpretation.new("prep-from-town").new_expression("@{prep-from} @{town}", prep_from_town_array)
        aller_de_a << i_prep_from_town
        alias_from = Alias.new(i_prep_from_town, "from")

        prep_to_town_array = [ alias_prep_to, alias_town ]
        i_prep_to_town = Interpretation.new("prep-to-town").new_expression("@{prep-to} @{town}", prep_to_town_array)
        aller_de_a << i_prep_to_town
        alias_to = Alias.new(i_prep_to_town, "to")

        i_go = Interpretation.new("go").new_textual(["go"])
        aller_de_a << i_go
        alias_go = Alias.new(i_go)

        go_from_to_array = [alias_go, alias_from, alias_to]
        i_go_from_to = Interpretation.new("go-from-to").new_expression("@{go} @{from} @{to}", go_from_to_array)
        aller_de_a << i_go_from_to
        alias_go_from_to = Alias.new(i_go_from_to)

        i_want = Interpretation.new("want").new_textual(["I want to"])
        aller_de_a << i_want
        alias_want = Alias.new(i_want)

        want_go_from_to_array = [ alias_want, alias_go_from_to ]
        i_want_go_from_to = Interpretation.new("want-go-from-to").new_expression("@{want} @{go-from-to}", want_go_from_to_array)
        aller_de_a << i_want_go_from_to

        # ecriture du package dans le repertoire de feed
        aller_de_a.to_file(importDir)

        Nls.restart

        # creation de la requete
        request = json_interpret_body(aller_de_a.id, "I want to go from Barcelona to New York", "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5")

        # execution de la requete
        actual = Nls.interpret(request)

        expected =
        {
          "interpretations" =>
            [
            {
            "package" => aller_de_a.id.to_s,
            "id" => i_want_go_from_to.id.to_s,
            "slug" => "want-go-from-to",
            "score" => 1.0
            }
            ]
        }

        assert_equal expected, actual

      end

      def test_interpret_recursive

        Interpretation.default_locale = "en-GB"

        # creation du feed d'init
        go_town = Package.new("go-town")

        i_new_york = Interpretation.new("new-york").new_textual(["New York", "NYC"])
        go_town << i_new_york
        alias_newyork = Alias.new(i_new_york)

        i_barca = Interpretation.new("barcelona").new_textual(["Barcelona", "Barca"])
        go_town << i_barca
        alias_barca = Alias.new(i_barca)

        i_paris = Interpretation.new("paris").new_textual(["Paris", "Capital of France"])
        go_town << i_paris
        alias_paris = Alias.new(i_paris)

        i_town = Interpretation.new("town").new_expression("@{new-york}", [alias_newyork])
        i_town.new_expression("@{barcelona}", [alias_barca])
        i_town.new_expression("@{paris}", [alias_paris])
        go_town << i_town
        alias_town = Alias.new(i_town)

        i_towns = Interpretation.new("towns")
        alias_towns = Alias.new(i_towns)
        i_towns.new_expression("@{town} @{towns}", [alias_town, alias_towns])
        i_towns.new_expression("@{town}", [alias_town])
        go_town << i_towns

        i_go = Interpretation.new("go").new_textual(["go to"])
        go_town << i_go
        alias_go = Alias.new(i_go)

        i_go_town = Interpretation.new("go-town").new_expression("@{go} @{towns}",[alias_go, alias_towns])
        go_town << i_go_town
        alias_go = Alias.new(i_go_town)

        # ecriture du package dans le repertoire de feed
        go_town.to_file(importDir)

        Nls.restart

        # creation de la requete
        request = json_interpret_body(go_town.id, "go to Paris New York Barcelona", "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5")

        # execution de la requete
        actual = Nls.interpret(request)

        expected =
        {
          "interpretations" =>
          [
          {
          "package" => go_town.id.to_s,
          "id" => i_go_town.id.to_s,
          "slug" => "go-town",
          "score" => 1.0
          }
          ]
        }

        assert_equal expected, actual

      end

      def test_builtin_features
        Interpretation.default_locale = "en-GB"

        # creation des feeds d'init
        pg_building_feature = Package.new("pg-building-feature")

        i_sea_view = Interpretation.new("sea-view").new_textual(["sea view", "sea front", "ocean view", "ocean front"])
        pg_building_feature << i_sea_view
        alias_sea_view = Alias.new(i_sea_view)

        i_swimming_pool = Interpretation.new("swimming-pool").new_textual(["swimming pool", "pool"])
        pg_building_feature << i_swimming_pool
        alias_swimming_pool = Alias.new(i_swimming_pool)

        i_building_feature = Interpretation.new("building-feature").new_expression("@{swimming-pool}",[alias_swimming_pool])
        i_building_feature.new_expression("@{sea-view}",[alias_sea_view])
        pg_building_feature << i_building_feature
        alias_building_feature = Alias.new(i_building_feature)

        i_building_features = Interpretation.new("building-features")
        alias_building_features = Alias.new(i_building_features)
        i_building_features.new_expression("@{building-feature} @{building-features}", [alias_building_feature, alias_building_features])
        i_building_features.new_expression("@{building-feature}", [alias_building_feature])
        pg_building_feature << i_building_features

        i_preposition_building_feature= Interpretation.new("preposition-building-feature").new_textual(["with", "at"])
        pg_building_feature << i_preposition_building_feature
        alias_preposition_building_feature = Alias.new(i_preposition_building_feature)

        pg_building_feature_array = [alias_preposition_building_feature, alias_building_features]
        i_pg_building_feature = Interpretation.new("pg-building-feature").new_expression("@{preposition-building-feature} @{building-features}", pg_building_feature_array)
        i_pg_building_feature.new_expression("@{building-features}", [alias_building_features])
        pg_building_feature << i_pg_building_feature
        alias_pg_building_feature = Alias.new(i_pg_building_feature)

        pg_building_feature.to_file(importDir)

        want_hotel = Package.new("want-hotel")

        i_want = Interpretation.new("want").new_textual(["I want", "I need", "I search", "I'm looking for"])
        want_hotel << i_want
        alias_want = Alias.new(i_want)

        i_hotel = Interpretation.new("hotel").new_textual(["an hotel"])
        want_hotel << i_hotel
        alias_hotel = Alias.new(i_hotel)

        i_want_hotel = Interpretation.new("want-hotel").new_expression("@{want} @{hotel}", [alias_want, alias_hotel])
        want_hotel << i_want_hotel
        alias_want_hotel = Alias.new(i_want_hotel)

        i_want_hotel_with_feature = Interpretation.new("want-hotel-with-feature").new_expression("@{want-hotel} @{pg-building-feature}", [alias_want_hotel, alias_pg_building_feature])
        want_hotel << i_want_hotel_with_feature

        want_hotel.to_file(importDir)

        puts "want hotel\n"

        # ap want_hotel.to_h

        puts "building feature\n"

        # ap pg_building_feature.to_h

        Nls.restart

        puts "\n\nrequest \n\n"
        request = json_interpret_body([want_hotel.id.to_s, pg_building_feature.id.to_s], "I want an hotel with swimming pool with sea view", "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5")

        # ap request

        puts "\n\nresult\n\n"

        actual = Nls.interpret(request)

        # ap actual

        expected =
        {
          "interpretations" =>
          [
          {
          "package" => want_hotel.id.to_s,
          "id" => i_want_hotel_with_feature.id.to_s,
          "slug" => "want-hotel-with-feature",
          "score" => 1.0
          }
          ]
        }

        assert_equal expected, actual

      end
    end
  end

end
