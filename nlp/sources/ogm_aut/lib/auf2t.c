/*
 *	Transfert d'un format auf en format aut.
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : June 2009
 *	Version 1.0
*/
#include "ogm_aut.h"



PUBLIC(int) OgAufToAut(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
unsigned char out[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;

if ((retour=OgAufScanf(handle,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    IFE(retour);
    IFE(OgAutAdd(handle,iout,out));
    }
  while((retour=OgAufScann(handle,&iout,out,nstate0,&nstate1,states)));
  }

DPcFree(ctrl_aut->Fstate);
DPcFree(ctrl_aut->Gstate);

DONE;
}


