/*
 *  Handling replacement variables.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : July 2005
 *  Version 1.0
*/
#include "ogm_xml.h"



STATICF(int) GetRevar(pr_(struct og_ctrl_xml *) pr(struct revar **));




int FindRevar(ctrl_xml,iname,name,pIrevar)
struct og_ctrl_xml *ctrl_xml;
int iname; unsigned char *name;
int *pIrevar;
{
int i,found=0;
struct revar *revar;

for (i=0; i<ctrl_xml->RevarUsed; i++) {
  revar = ctrl_xml->Revar + i;
  if (revar->iname != iname) continue;
  if (memcmp(revar->name,name,iname)) continue;
  found=1; *pIrevar=i; break;
  }

return(found);
}




int AddRevar(ctrl_xml,name,value)
struct og_ctrl_xml *ctrl_xml;
unsigned char *name, *value;
{
struct revar *revar;
int Irevar,iname,ivalue;

IFE(Irevar=GetRevar(ctrl_xml,&revar));
iname=strlen(name);
if (iname>DOgMaxRevarSize-1) iname=DOgMaxRevarSize-1;
memcpy(revar->name,name,iname); revar->name[iname]=0;
revar->iname = iname;

ivalue=strlen(value);
if (ivalue>DOgMaxRevarSize-1) ivalue=DOgMaxRevarSize-1;
memcpy(revar->value,value,ivalue); revar->value[ivalue]=0;
revar->ivalue = ivalue;

return(Irevar);
}





STATICF(int) GetRevar(ctrl_xml,prevar)
struct og_ctrl_xml *ctrl_xml;
struct revar **prevar;
{
int i = ctrl_xml->RevarNumber;
struct revar *revar = 0;

beginGetRevar:

if (ctrl_xml->RevarUsed < ctrl_xml->RevarNumber) {
  i = ctrl_xml->RevarUsed++;
  revar = ctrl_xml->Revar + i;
  }

if (i == ctrl_xml->RevarNumber) {
  char erreur[DOgErrorSize];
  unsigned a, b; struct revar *og_revar;
  if (ctrl_xml->loginfo->trace & DOgXmlTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_xml->loginfo->where, 0
                , "GetRevar: max revar number (%d) reached"
                , ctrl_xml->RevarNumber);
    }
  a = ctrl_xml->RevarNumber; b = a + (a>>2) + 1;
  IFn(og_revar=(struct revar *)malloc(b*sizeof(struct revar))) {
    sprintf(erreur,"GetRevar: malloc error on Revar for %d structures and %lu bytes", b, b*sizeof(struct revar));
    OgErr(ctrl_xml->herr,erreur); DPcErr;
    }
  memcpy( og_revar, ctrl_xml->Revar, a*sizeof(struct revar));

  DPcFree(ctrl_xml->Revar); ctrl_xml->Revar = og_revar;
  ctrl_xml->RevarNumber = b;

  if (ctrl_xml->loginfo->trace & DOgXmlTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_xml->loginfo->where, 0
                , "GetRevar: new revar number is %d", ctrl_xml->RevarNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur, "GetRevar: no more revar max is %d", ctrl_xml->RevarNumber);
  OgErr(ctrl_xml->herr,erreur); DPcErr;
#endif

  goto beginGetRevar;
  }

memset(revar,0,sizeof(struct revar));

if (prevar) *prevar = revar;
return(i);
}



