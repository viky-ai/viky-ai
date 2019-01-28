# Convention de nommage des interprétations

Avec l'experience nous pouvons commencer à préparer des règle de nommage des interprétation et des entités

 * Les différents noms des interpretations/entities seront écrits en anglais, sauf besoin explicite, à voir selon client
 
 * Les interprétation contenant une liste seront préfixées par "list" (en anglais). ex: list\_criteres

    selon la langue, celà peut être un suffixe. ex en anglais : criteria\_list

 * Les listes sont remplies avec des éléments.
    
    Ces éléments sont des interprétations.

    Ces interpretations doivent être préfixées par "element" (en anglais). ex: element\_critere ou post-fixée en anglais: criteria\_element

>>>
Exemple : 

Une liste de couleurs sera constituée de 2 interprétations : 

  * Une première colour\_element, qui contiendra les couleurs et leurs informations (nom, abbréviation et code RGB par exemple...)

  * Une seconde colour\_list, qui sera une liste de colour\_element

>>>

  * On peut distinguer deux types d'expression : les interprétations qui expriment le concept lui-même et les interprétations qui sont des contenus de ce concept.

    Les interprétations exprimant le concept lui-même seront au singulier, les interprétation de contenu du contexte seront au pluriel

>>>
Exemple: 

Les mots maîtrise, compétence, connaissance, expertise, capacité, exprime le concept de compétence mais pas une competence, l'interprétation sera donc nommée "competence"

Les mots "bien organisé" "autonomie", "développement" "conception assistée par ordinateur" sont des compétences, l'interprétation sera donc nommée "competences"
>>>
    
  * Les articles prépositions et conjonctions ne seront pas spécifiées, sauf besoin explicite de différenciation. 

>>>
Exemple : 

Dans la phrase "je sais faire du développement", "sais faire" sera identifié comme compétence et "développement" sera identifié comme compétences, mais le "du" ne sera pas identifié.

A l'inverse, si on prend l'agent imdb, un film DE brad pitt et un film AVEC brad pitt ne sera pas interprété de la même manière, dans un cas brad pitt est réalisateur, dans le second il est acteur, ici le besoin est bel et bien explicite.
>>>

  * Les éléments de ponctuation ne seront pas spécifiées, sauf besoin explicite (identification de séparateur, etc...)

A priori, dans le cas de Pertimm, et comme les agents sont généralement multi-lingues, la dénomination se devrait d'être anglaise, au moins dans tous les usages faits par Pertimm. Ce qui simplifiera la lecture : on ne devra pas jongler entre list_criteres et criteria_list ou element_critere ou criterium_element.
