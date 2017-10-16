/*
 *	Test d'existence d'une ligne de caract\`eres dans l'automate.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : D\'ecembre 1996, F\'evrier 1997, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


/*
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Renvoit l'\'etat en fin de cha\^ine,
 *	ou 0 si la cha\^ine n'existe pas dans l'automate.
 *	Cette fonction est optimis\'ee en vitesse.
*/

oindex AufGline2(af,line)
struct fstate *af; int *line;
{
register int i=0;
register oindex state=1;
etiquette:
if (line[i]==(-1)) return(state);
do if (af[state].lettre==(unsigned)line[i])
    { state=af[state].index; i++; goto etiquette; }
while(!af[state++].last);
return((oindex)0);
}




oindex AugGline(ag,line)
struct gstate *ag; int *line;
{
register int i=0;
register oindex state=1;
etiquette:
if (line[i]==(-1)) return(state);
do if (ag[state].lettre==(unsigned)line[i])
    { state=ag[state].index; i++; goto etiquette; }
while(!ag[state++].last);
return((oindex)0);
}

