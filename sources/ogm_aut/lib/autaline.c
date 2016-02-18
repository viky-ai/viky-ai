/*
 *	Ajout d'une ligne de caract\`eres dans l'automate.
 *	Copyright (c) 1996-2006 Patrick Constant
 *	Dev : Avril,Juillet 1996, February 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


/*
 *	Ajoute une ligne de "lettres" dans l'automate.
 *	Cette ligne correspond \`a un "mot" de l'automate.
 *	la ligne est une suite d'entiers qui correspond 
 *	\`a un codage d'une ligne (ISO-latin-1, Unicode ou autres).
*/

int AutAline2(ctrl_aut,line)
struct og_ctrl_aut *ctrl_aut; 
int *line;
{
int i,state,nstate;
IFn(ctrl_aut->StateUsed) { IFE(state=AllocState(ctrl_aut)); } else state=1;

for (i=0; line[i]!=(-1); i++) {
  IFn(nstate=AutGet2(ctrl_aut,line[i],state)) {
    IFE(nstate=AllocState(ctrl_aut));
    IFE(AutPut2(ctrl_aut,line[i],state,nstate));
    }
  state=nstate;
  }
if (ctrl_aut->frequency && ctrl_aut->State[state].freq<63) ctrl_aut->State[state].freq++;
ctrl_aut->State[state].final=1;
ctrl_aut->State[state].start=1;
DONE;
}

