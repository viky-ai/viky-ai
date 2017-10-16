/*
 *	Miscellanous functions  
 *	Copyright (c) 2009-2011 Pertimm by Patrick Constant
 *	Dev: July 2009, February 2011
 *	Version 1.2
*/
#include "ogm_aut.h"





PUBLIC(int) OgAutGetType(handle,ptype)
void *handle; int *ptype;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int type=0;

if (ctrl_aut->State) type |= DOgAutTypeAut;
if (ctrl_aut->Fstate) type |= DOgAutTypeAuf;
if (ctrl_aut->Gstate) type |= DOgAutTypeAug;

*ptype = type;

DONE;
}


