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
    end
  end

end
