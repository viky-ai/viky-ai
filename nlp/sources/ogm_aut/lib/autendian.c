/*
 *	Handling little endian and big endian.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev: October 2007
 *	Version 1.0
*/

#include "ogm_aut.h"




PUBLIC(int) OgAutEndian(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
IFE(AutEndianFstate(ctrl_aut));
IFE(AutEndianState(ctrl_aut));
IFE(AutEndianGstate(ctrl_aut));
IFE(AutEndianFrstate(ctrl_aut));
DONE;
}




int AutEndianChangeUnsigned(ctrl_aut,buffin,buffout)
struct og_ctrl_aut *ctrl_aut;
unsigned buffin,*buffout;
{
*buffout=0;
*buffout|=(buffin & 0xff) << 24;
*buffout|=(buffin & 0xff00) << 8;
*buffout|=(buffin & 0xff0000) >> 8;
*buffout|=(buffin & 0xff000000) >> 24;
DONE;
}



