/*
 *	Ajout d'une transition entre deux \'etats.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : Mars,Avril,Juillet 1996, February 2006
 *	Version 1.4
*/

#include "ogm_aut.h"


/*	PUBLIC
 *	Met la transition allant de l'\'etat 'start' vers
 *	l'\'etat 'end' qui passe par la lettre 'lettre'.
 *	Les \'etats sont allou\'es. On ne v\'erifie
 *	pas si le code de la lettre exc\`ede 2^DPcMxlettre.
*/

PUBLIC(int) AutPut2(ctrl_aut,lettre,start,end)
struct og_ctrl_aut *ctrl_aut;
oindex lettre,start,end;
{
oindex new,state,bfstate;
IFn(ctrl_aut->State[start].index) {
  ctrl_aut->State[start].lettre=lettre;
  ctrl_aut->State[start].index=end;
  ctrl_aut->State[start].start=1;
  DONE;
  }
IFE(new=AllocState(ctrl_aut));
state=start;
do {
  /** On trie selon les lettres pour pouvoir minimiser **/
  /** correctement des automates construits \`a partir **/
  /** de fichiers non tri\'es **/
  if (lettre<ctrl_aut->State[state].lettre) {
    /** Le transfert n'est pas si \'evident pour plusieurs raisons : **/
    /**   - la premi\`ere transition doit rester la m\^eme car **/
    /**     un autre \'etat pointe dessus (on pourrait le retrouver) **/
    /**   - les champs start et final doivent rester correct sur **/
    /**     cette transition (le memcpy assure cela) **/
    ctrl_aut->State[new]=ctrl_aut->State[state];
    ctrl_aut->State[state].next=new; ctrl_aut->State[state].index=end; ctrl_aut->State[state].lettre=lettre;
    ctrl_aut->State[new].start=0; ctrl_aut->State[new].final=0;
    DONE;
    }
  bfstate=state;
  }
while(GnState(state));

ctrl_aut->State[new].index=end;
ctrl_aut->State[new].lettre=lettre;
ctrl_aut->State[bfstate].next=new;

DONE;
}

