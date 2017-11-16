/*
 *  Handling suggestions (used for quality assessments)
 *  Copyright (c) 2010 Pertimm, Inc. by Patrick Constant
 *  Dev : February 2010
 *  Version 1.0
*/
#include "ogm_ltras.h"



PUBLIC(og_status) OgLtrasSuggestionInit(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
IFE(OgAutReset(ctrl_ltras->ha_suggest));
ctrl_ltras->nb_suggestions=0;
DONE;
}





PUBLIC(og_status) OgLtrasSuggestionAdd(void *handle, int is, unsigned char *s)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
int ibuffer; unsigned char *p,buffer[DPcPathSize];
memcpy(buffer,s,is); ibuffer=is;
buffer[ibuffer++]=0; buffer[ibuffer++]=1;
p = buffer+ibuffer;
OggNout(ctrl_ltras->nb_suggestions,&p);
ibuffer=p-buffer;
IFE(OgAutAdd(ctrl_ltras->ha_suggest,ibuffer,buffer));
ctrl_ltras->nb_suggestions++;
DONE;
}




int LtrasSuggestionGet(void *handle, int is, unsigned char *s, int *psuggestion_number)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
int ibuffer; unsigned char *p,buffer[DPcPathSize];
unsigned char out[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;
int suggestion_number;

memcpy(buffer,s,is); ibuffer=is;
buffer[ibuffer++]=0; buffer[ibuffer++]=1;

if ((retour=OgAutScanf(ctrl_ltras->ha_suggest,ibuffer,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do {
    p=out; IFE(DOgPnin4(ctrl_ltras->herr,&p,&suggestion_number));
    *psuggestion_number = suggestion_number;
    return(1);
    }
  while((retour=OgAutScann(ctrl_ltras->ha_suggest,&iout,out,nstate0,&nstate1,states)));
  }
return(0);
}


