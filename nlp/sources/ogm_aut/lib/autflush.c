/*
 *	Flushing complet d'un automate.
 *	Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *	Dev: Mars 1997, February 2006
 *	Version 1.1
*/

#include "ogm_aut.h"


/*
 *	Flushing de la m\'emoire associ\'ee \`a un automate
 *	dont le handle est 'handle'. On lib\`ere les trois automates
 *	s'ils existent et on d\'esactive le handle.
*/

PUBLIC(int) OgAutFlush(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut;

IFn(handle) DONE;

ctrl_aut = (struct og_ctrl_aut *)handle;
DPcFree(ctrl_aut->State);
DPcFree(ctrl_aut->FreeState);
DPcFree(ctrl_aut->Lstmin);
if (ctrl_aut->FstateAllocated) {
  DPcFree(ctrl_aut->Fstate);
  }
DPcFree(ctrl_aut->Gstate);

DPcFree(ctrl_aut);
DONE;
}




