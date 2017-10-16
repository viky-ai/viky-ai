/*
 *	Test d'existence d'une ligne de caract\`eres dans l'automate.
 *	Copyright (c) 1996-2006	Pertimm by Patrick Constant
 *	Dev : Avril,Juillet 1996, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


/*
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Renvoit l'\'etat en fin de cha\^ine,
 *	ou 0 si la cha\^ine n'existe pas dans l'automate.
*/

oindex AutGline2(ctrl_aut,line)
struct og_ctrl_aut *ctrl_aut;
int *line;
{
int i;
oindex nstate,state=1;
for (i=0; line[i]!=(-1); i++) {
  IFn(nstate=AutGet2(ctrl_aut,line[i],state)) {
#ifdef DEVERMINE
    int j;
    printf("\nAutGline from ");
    for (j=0; line[j]!=(-1); j++)
      if (j==i-1) printf("%d.",line[j]);
      else printf("%d ",line[j]);
    printf(": state %d\n",state);
#endif
    return((oindex)0);
    }
  state=nstate;
  }
return(state);
}


