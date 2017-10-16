/*
 *	Indique que l'\'etat est final ou non.
 *	Copyright (c) 1996-2006 Patrick Constant
 *	Dev : Avril,Juillet 1996, March, April 2006
 *	Version 1.2
*/

#include "ogm_aut.h"





PUBLIC(int) OgAutIsFinal(handle,state)
void *handle; oindex state;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
if (ctrl_aut->State[state].final) return(1);
return(0);
}




/*
 *	permet de dire si l'\'etat 'state' est final (value==1)
 *	ou non (value==0). Si value > 1, n'affecte pas l'\'etat.
 *	renvoit 1 si l'\'etat est final et 0 sinon.
*/

int AutFinal2(ctrl_aut,state,value)
struct og_ctrl_aut *ctrl_aut;
oindex state; int value;
{
if (value==0) ctrl_aut->State[state].final=0;
else if (value==1) ctrl_aut->State[state].final=1;
return(ctrl_aut->State[state].final);
}



