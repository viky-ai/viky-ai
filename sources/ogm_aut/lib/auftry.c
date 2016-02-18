/*
 *  Regarde si une ligne de "lettres" est dans l'automate.
 *  Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *  Dev : D\'ecembre 1996, Septembre 1998, March 2006
 *  Version 1.2
*/

#include "ogm_aut.h"


/*  PUBLIC
 *  Regarde si une ligne de "lettres" est dans l'automate.
 *  Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *  Renvoit 1 si la cha\^ine existe mais n'est pas finale (i.e.
 *  la cha\^ine est une sous-cha\^ine d'une cha\^ine finale
 *  dans l'automate.
 *  Renvoit 2 si la cha\^ine est une cha\^ine finale.
*/

PUBLIC(int) OgAufTry(handle,iline,line)
void *handle; int iline; unsigned char *line;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int out[DPcAutMaxBufferSize];
char erreur[DOgErrorSize];
int state;

IF(AutTranslateBuffer2(iline,line,out)) return(0);

if (ctrl_aut->Fstate) {
  IFn(state=AufGline2(ctrl_aut->Fstate,out)) return(0);
  else if (ctrl_aut->Fstate[state].final) return(2);
  else return(1);
  }
else if (ctrl_aut->Gstate) {
  IFn(state=AugGline(ctrl_aut->Gstate,out)) return(0);
  else if (ctrl_aut->Gstate[state].final) return(2);
  else return(1);
  }
return(0);
}


