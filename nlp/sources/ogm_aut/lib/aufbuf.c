/*
 *	Reading and writing an automaton in a buffer
 *	Copyright (c) 2008 Pertimm by Patrick Constant
 *	Dev : July 2008
 *	Version 1.0
*/
#include "ogm_aut.h"




PUBLIC(int) OgAufBufferSize(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
return(sizeof(int)+ctrl_aut->FstateUsed*sizeof(struct fstate));
}





PUBLIC(int) OgAufBufferRead(handle,ibuffer,buffer)
void *handle; int ibuffer;
char *buffer;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char erreur[DOgErrorSize];
int size;

if (ctrl_aut->FstateAllocated) {
  DPcFree(ctrl_aut->Fstate);
  }
ctrl_aut->FstateAllocated=0;
memcpy(&ctrl_aut->FstateUsed,buffer,sizeof(int));
size = sizeof(int) + ctrl_aut->FstateUsed*sizeof(struct fstate);
if (ibuffer != size) {
  sprintf(erreur,"OgAufBufferRead (%s): ibuffer (%d) != size (%d) "
    , ctrl_aut->name, ibuffer, size);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
ctrl_aut->Fstate = (struct fstate *)(buffer+sizeof(int));
DONE;
}





PUBLIC(int) OgAufBufferWrite(handle,ibuffer,buffer)
void *handle; int ibuffer;
char *buffer;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char erreur[DOgErrorSize];
int size;

memcpy(buffer,&ctrl_aut->FstateUsed,sizeof(int));
size = sizeof(int) + ctrl_aut->FstateUsed*sizeof(struct fstate);
if (ibuffer != size) {
  sprintf(erreur,"OgAufBufferRead (%s): ibuffer (%d) != size (%d) "
    , ctrl_aut->name, ibuffer, size);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
memcpy(buffer+sizeof(int), ctrl_aut->Fstate, ctrl_aut->FstateUsed*sizeof(struct fstate));
DONE;
}



