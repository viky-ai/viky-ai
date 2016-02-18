/*
 *  Getting an HTTP header line value specifically
 *  Copyright (c) 2007 Pertimm by Patrick Constant
 *  Dev : March,September 2007
 *  Version 1.1
*/
#include "ogm_http.h"





PUBLIC(int) OgHttpHeaderScanValues(handle,hh,line,func,ptr)
void *handle; struct og_http_header2 *hh; int line;
int (*func)(pr_(void *) pr_(int) pr(unsigned char *));
void *ptr;
{
struct og_ctrl_http *ctrl_http = (struct og_ctrl_http *)handle;
unsigned char out[DPcAutMaxBufferSize+9];
int iout,retour,nstate0,nstate1,stop;
oindex states[DPcAutMaxBufferSize+9];
char buffer[DOgHttpHeaderLineSize];

sprintf(buffer,"%d:",line);

if ((retour=OgAutScanf(hh->ha_header,-1,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do {
    IFE(retour);
    IFE(stop=(*func)(ptr,iout,out));
    if (stop) break;
    }
  while((retour=OgAutScann(hh->ha_header,&iout,out,nstate0,&nstate1,states)));
  }

DONE;
}




