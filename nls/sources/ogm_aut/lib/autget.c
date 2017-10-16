/*
 *	Suivre une lettre \`a partir d'un \'etat.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : Mars,Juillet 1996, February 2006
 *	Version 1.2
*/

#include "ogm_aut.h"





oindex AutGet2(ctrl_aut,lettre,start)
struct og_ctrl_aut *ctrl_aut;
oindex lettre,start;
{
oindex state=start;
do {
  if (ctrl_aut->State[state].lettre==lettre) return(ctrl_aut->State[state].index);
  }
while(GnState(state));
return((oindex)0);
}




