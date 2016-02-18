/*
 *	Scanning extended strings (or rupture strings), 
 *  based upon OgAutScanf and OgAutScann 
 *	Copyright (c) 1998-2008 Pertimm by Patrick Constant
 *	Dev : July 2008
 *	Version 1.0
*/
#include "ogm_aut.h"


STATICF(int) AutExttoRup(pr_(struct og_ctrl_aut *) pr_(int) pr_(int *) pr_(int *) pr_(int *) pr(unsigned char *));



PUBLIC(int) OgAutScanfRup(handle,iline,rupture,line,iout,prupture,out,nstate0,nstate1,states)
void *handle; int iline,rupture; unsigned char *line;
int *iout,*prupture; unsigned char *out;
int *nstate0,*nstate1; oindex *states;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int il,l[DPcAutMaxBufferSize+9];
int io,o[DPcAutMaxBufferSize+9];
int retour;

IFE(OgAutRupToExt(ctrl_aut,iline,rupture,line,&il,l));
IFE(retour=OgAutScanfExt(handle,il,l,&io,o,nstate0,nstate1,states));
IFE(AutExttoRup(ctrl_aut,io,o,iout,prupture,out));

return(retour);
}




PUBLIC(int) OgAutScannRup(handle,iout,prupture,out,nstate0,nstate1,states)
void *handle; int *iout,*prupture; unsigned char *out;
int nstate0,*nstate1; oindex *states;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int io,o[DPcAutMaxBufferSize+9];
int retour;

IFE(retour=OgAutScannExt(handle,&io,o,nstate0,nstate1,states));
IFE(AutExttoRup(ctrl_aut,io,o,iout,prupture,out));

return(retour);
}




STATICF(int) AutExttoRup(ctrl_aut,io,o,iout,prupture,out)
struct og_ctrl_aut *ctrl_aut; int io,*o;
int *iout,*prupture; unsigned char *out;
{
int i,j;

*prupture=(-1);
for (i=j=0; i<io; i++) {
  if (o[i]==0) {
    *prupture=i;
    }
  else {
    out[j++]=o[i]-1;
    }
  }
*iout=j;
DONE;
}




PUBLIC(int) OgAutScanfExt(handle,iline,line,iout,out,nstate0,nstate1,states)
void *handle; int iline; int *line; int *iout; int *out;
int *nstate0,*nstate1; 
oindex *states;
{
struct og_ctrl_aut *ctrl_aut;
int i,nstate0i,nstate1i,istates;
IFn(handle) return(0);
ctrl_aut = (struct og_ctrl_aut *)handle;
*nstate0=0; *nstate1=0; *iout=0; out[0]=0;
IFn(ctrl_aut->StateUsed) return(0);
IF(OgAutCheckLine(ctrl_aut,"OgAutScanfExt",iline,line)) return(0);
IFn(nstate0i=AutGlineState(ctrl_aut,line,states,&istates)) { *nstate0=istates; return(0); }
nstate1i=AutScanf1(ctrl_aut,nstate0i,states);
for (i=nstate0i-1; i<nstate1i-1; i++)
  out[i-nstate0i+1]=ctrl_aut->State[states[i]].lettre; out[i-nstate0i+1]=0;
IFx(iout) *iout=nstate1i-nstate0i;
*nstate0=nstate0i;
*nstate1=nstate1i;
return(1);
}




PUBLIC(int) OgAutScannExt(handle,iout,out,nstate0,nstate1,states)
void *handle; int *iout; int *out;
int nstate0,*nstate1;
oindex *states;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int i,retour;
IFE(retour=AutScann1(ctrl_aut,nstate0,nstate1,states));
for (i=nstate0-1; i<(*nstate1)-1; i++)
  out[i-nstate0+1]=ctrl_aut->State[states[i]].lettre; out[i-nstate0+1]=0;
IFx(iout) *iout=(*nstate1)-nstate0;
return(retour);
}


