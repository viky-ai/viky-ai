/*
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Copyright (c) 1996-2006 Pertimm, Inc. by Patrick Constant
 *	Dev: Avril,Juillet 1996, September 1998, February 2002, March 2006
 *	Version 1.4
*/

#include "ogm_aut.h"


/*	PUBLIC
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *	Renvoit 1 si la cha\^ine existe mais n'est pas finale (i.e. 
 *	la cha\^ine est une sous-cha\^ine d'une cha\^ine finale
 *	dans l'automate.
 *	Renvoit 2 si la cha\^ine est une cha\^ine finale.
*/

PUBLIC(int) OgAutTry(handle,iline,line)
void *handle; int iline; unsigned char *line;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int out[DPcAutMaxBufferSize];
int state;

if (ctrl_aut->StateUsed<=1) return(0);
IF(AutTranslateBuffer2(iline,line,out)) return(0);
IFn(state=AutGline2(handle,out)) return(0);
else if (AutFinal2(handle,state,2)) return(2);
else return(1);
}


