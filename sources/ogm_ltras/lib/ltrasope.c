/*
 *  Handling operations (used for quality assessments)
 *  Copyright (c) 2010 Pertimm, Inc. by Lois Rigouste
 *  Dev : April 2010
 *  Version 1.0
*/
#include "ogm_ltras.h"



PUBLIC(og_status) OgLtrasOperationInit(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
IFE(OgAutReset(ctrl_ltras->ha_oper));
ctrl_ltras->nb_operations=0;
DONE;
}





PUBLIC(int) OgLtrasOperationGet(void *handle, int is, unsigned char *s)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
int ibuffer; unsigned char *p,buffer[DPcPathSize];
unsigned char out[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;
int operation_number;

memcpy(buffer,s,is); ibuffer=is;
buffer[ibuffer++]='|';

// if operation already exists in automaton, only get its number
// otherwise add it
if ((retour=OgAutScanf(ctrl_ltras->ha_oper,ibuffer,buffer,&iout,out,&nstate0,&nstate1,states))) {
  p=out; IFE(DOgPnin4(ctrl_ltras->herr,&p,&operation_number));
  ctrl_ltras->current_op = operation_number;
  }
else{
  if(ctrl_ltras->nb_operations == DOgLtrasMaxNbOperations){
  ctrl_ltras->current_op = DOgLtrasMaxNbOperations-1;
  DONE;
    }
  p = buffer+ibuffer;
  OggNout(ctrl_ltras->nb_operations,&p);
  ctrl_ltras->current_op = ctrl_ltras->nb_operations;
  ibuffer=p-buffer;
  IFE(OgAutAdd(ctrl_ltras->ha_oper,ibuffer,buffer));
  ctrl_ltras->nb_operations++;
  }

DONE;
}


