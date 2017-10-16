/*
 *	Module de gestion de la fr\'equence d'ajout sur les cha\^ines.
 *	Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *	Dev : Mars 1997, March 2006
 *	Version 1.1
*/
#include "ogm_aut.h"



/*
 *	Fonction qui permet d'indiquer \`a la fonction d'ajout
 *	de comptabiliser la fr\'equence d'ajout de cha\^ines de 
 *	caract\`eres (nombre de fois o\`u la m\^eme cha\^ine
 *	a \'et\'e ajout\'ee). si 'value' vaut z\'ero (valeur par d\'efaut)
 *	il n'y a pas comptabilisation sinon il y a comptabilisation.
*/

PUBLIC(int) OgAutFreqCtrl(handle,value)
void *handle; int value;
{
struct og_ctrl_aut *ctrl_aut;
IFn(handle) DONE;
ctrl_aut = (struct og_ctrl_aut *)handle;
ctrl_aut->frequency=value;
return(0);
}



/*
 *	Fonction qui renvoit la fr\'equence d'un \'etat donn\'e.
 *	Cette fr\'equence est mise \`a jour sur les \'etats finaux
 *	et permet de dire combien de fois une cha\^ine de caract\`eres
 *	a \'et\'e ajout\'ee.
*/

PUBLIC(int) OgAutFreq(handle,state)
void *handle; oindex state;
{
struct og_ctrl_aut *ctrl_aut;
IFn(handle) DONE;
ctrl_aut = (struct og_ctrl_aut *)handle;
return(ctrl_aut->State[state].freq);
}


