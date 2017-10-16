/*
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : D\'ecembre 1996, Septembre 1998, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


/*	PUBLIC
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *	Renvoit 1 si la cha\^ine existe mais n'est pas finale,
 *	Renvoit 2 si la cha\^ine est une cha\^ine finale.
 *    dans ces deux derniers cas, renvoit dans 'letters'
 *	  la liste des lettres qui suivent cette cha\^ine.
*/

PUBLIC(int) OgAutLet(handle,iline,line,iletters,letters)
void *handle; int iline; unsigned char *line;
int *iletters; unsigned char*letters;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int out[DPcAutMaxBufferSize];
int i,state,retour;
IF(AutTranslateBuffer2(iline,line,out)) return(0);
IFn(state=AutGline2(ctrl_aut,out)) return(0);
else if (AutFinal2(ctrl_aut,state,2)) retour=2; else retour=1;
i=0; do letters[i++]=ctrl_aut->State[state].lettre;
while(GnState(state));
letters[i]=0;
IFx(iletters) *iletters=i;
return(retour);
}


