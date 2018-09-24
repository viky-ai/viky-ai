# convention de nommage des interprétations

Avec l'experience nous pouvons commencer à préparer des règle de nommage des interprétation et des entités

* Les différents noms des interpretations/entities seront écrits en anglais, sauf besoin explicite, à voir selon client

* Les interprétation contenant une liste seront préfixées par "liste". ex: liste_criteres
selon la langue, celà peut être un suffixe. ex en anglais : criteria_list

* Les listes sont remplies avec des éléments.
Ces éléments sont des interprétations.
Ces interpretations doivent être préfixées par "element". ex: element_critere ou post-fixée en anglais: criteria_element

* On peut distinguer deux types d'expression : les expressions qui exprime le concept lui-même et les expressions qui sont des exemples de concept. Par exemple les mots maîtrise, compétence, connaissance, expertise, capacité, exprime le concept de compétence mais pas une competence, alors que "bien organisé" "autonomie", "développement" "conception assistée par ordinateur" sont des compétences. nous proposons la chose suivante : mot au singulier = les expressions de ce mot, mot au pluriel = les mots qui font référence au concept exprimé par le mot. Exemple : l'entity competence contiendra les mots maîtrise, compétence, connaissance, expertise et capacité, alors que l'entity competences contiendra les mots "bien organisé" "autonomie", "développement" "conception assistée par ordinateur"

* Les articles prépositions et conjonctions ne seront pas spécifiées, sauf besoin explicite de différenciation. par exemple : dans la phrase "je sais faire du développement", "sais faire" sera identifié comme compétence et "développement" sera identifié comme compétences, mais le "du" ne sera pas identifié. à l'inverse, si on prend l'agent imdb, un film DE brad pitt et un film AVEC brad pitt ne sera pas interprété de la même manière, dans un cas brad pitt est réalisateur, dans le second il est acteur, ici le besoin est bel et bien explicite.

* Les éléments de ponctuation ne seront pas spécifiées, sauf besoin explicite (identification de séparateur, etc...)

A priori, dans le cas de Pertimm, et comme les agents sont généralement multi-lingues, la dénomination se devrait d'être anglaise, au moins dans tous les usages faits par Pertimm. Ce qui simplifiera la lecture : on ne devra pas jongler entre liste_criteres et criteria_list ou element_critere ou criterium_element.
