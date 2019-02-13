# viky.ai - Agent Style Guide

Ce guide de style dédié aux agents de viky.ai recommande des bonnes pratiques conçues pour qu'un utilisateur de viky.ai puisse produire des agents maintenus par d'autres utilisateurs de la plateforme.

## Convention de nommage des agents

Les noms des agent sont en _anglais_ et doivent être relativement courts.
Lorsqu'il s'agit d'un agent réalisé pour un client ou un projet donné, celui-ci apparaît dans le nom de l'agent.
Lorsque vous souhaitez versionner un agent, indiquez la version dans le nom de celui-ci.

Globalement utilisez la syntaxe suivante : <code>(Client|Projet) (Nom de l'agent) (Version)</code>.

    # Bien
    Address
    PJ Demo v1.1

    # Mauvais
    Agent de reconnaissance d'adresses
    Démontrateur v 1.1 pour PagesJaunes


## Convention de nommage des listes d'entités

Les noms des listes d'entités sont _en anglais_ et _au pluriel_.

    # Bien
    colors
    brands
    towns

    # Mauvais
    color
    couleurs
    brand
    ville


## Convention de nommage des interprétations

### En anglais

Les noms des interprétations sont _en anglais_.

    # Bien
    colors
    dates
    date_ranges

    # Mauvais
    couleur
    intervalle_de_dates


### Singulier ou pluriel ?

On peut distinguer deux types d'interprétations, celles qui expriment le concept lui-même et les interprétations qui sont des contenus de ce concept.

Les interprétations exprimant le concept lui-même seront au singulier.
Les interprétation de contenu du contexte seront au pluriel.

_Exemples :_

Les mots « maîtrise », « compétence », « connaissance », « expertise », « capacité », exprime le concept de compétence mais pas une compétence, l'interprétation sera donc nommée « skill ».

Les mots « bien organisé », « autonomie », « développement », « conception assistée par ordinateur » sont des compétences, l'interprétation sera donc nommée « skills ».

### Interprétations avec l'option « List »

Les interprétations permettant de détecter une liste d'éléments (option "List") sont au pluriel et suffixées par **_list**.

    # Bien
    criteria_list
    colors_list

    # Mauvais
    critères
    criteria
    list_criteria
    color_list

Une liste est constituée d'éléments. Ces éléments peuvent être des interprétations ou des entités.

#### Utilisation directe d'une liste d'entités

Lorsque vous souhaitez reconnaitre une liste d'éléments de même nature, vous pouvez directement utiliser une liste d'entités. Prenons l'exemple suivant : vous souhaitez reconnaître une liste de couleurs. Pour cela, mettrez en place une interprétation nommée **colors_list** faisant référence à la liste d'entités **colors** rassemblant les couleurs avec leur informations associées (nom, abbréviation, code RGB, ...).

    # Bien
    # Interprétation List         Liste d'entités
    colors_list                   colors

    # Mauvais
    # Interprétation List         Liste d'entités
    colors                        color

#### Passage par une interprétation intermédiaire

Lorsque vous souhaitez reconnaitre une liste d'éléments de natures multiples, il est conseiller de passer par une interprétation intermédiaire. Dans ce cas  l'interprétation intermédiaire à le même nom que l'interprétation parente sans le suffixe **_list**.

Dans la phrase « je veux visiter Paris, Londres, l'Inde et la Franche-Comté », nous pouvons identifier une liste de lieux en utilisant une interprétation **locations_list** constituée d'une interprétation **locations** faisant référence à plusieurs listes d'entitées (countries, towns, areas, ...).

    # Bien
    # Interprétation List         Interprétation intermédiaire
    locations_list                locations

    # Mauvais
    # Interprétation List         Interprétation intermédiaire
    lieux_list                    lieux
    locations                     location
    locations_list                location_element

## Prépositions, conjonctions

Les articles prépositions et conjonctions ne sont pas spécifiées, sauf besoin explicite de différenciation.

_Exemples :_

Dans la phrase « je sais faire du développement», « sais faire » est identifié comme compétence (**skill**) et « développement » est identifié comme compétences (**skills**), mais la préposition « du » n'est pas identifiée.

Dans la phrase « un film de Brad Pitt et un film avec Brad Pitt », le premier « Brad Pitt » est réalisateur (**directors**), le second « Brad Pitt » il est acteur (**actors**). Ici le besoin est bel et bien explicite, les prépositions « de » et « avec » sont spécifiées.

## Ponctuation

Les éléments de ponctuation ne seront pas spécifiées, sauf besoin explicite (identification de séparateur, etc...).
