/*
 *	Initialization for ogm_xml functions
 *	Copyright (c) 2005 Pertimm by Patrick Constant
 *	Dev : July 2005
 *	Version 1.0
*/
#include "ogm_xml.h"





PUBLIC(void *) OgXmlInit(param)
struct og_xml_param *param;
{
char erreur[DOgErrorSize];
struct og_ctrl_xml *ctrl_xml;

IFn(ctrl_xml=(struct og_ctrl_xml *)malloc(sizeof(struct og_ctrl_xml))) {
  sprintf(erreur,"OgXmlInit: malloc error on ctrl_xml");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_xml,0,sizeof(struct og_ctrl_xml));

ctrl_xml->herr = param->herr;
ctrl_xml->hmutex = param->hmutex;
ctrl_xml->cloginfo = param->loginfo;
ctrl_xml->loginfo = &ctrl_xml->cloginfo;

ctrl_xml->RevarNumber = DOgRevarNumber;
IFn(ctrl_xml->Revar=(struct revar *)malloc(ctrl_xml->RevarNumber*sizeof(struct revar))) {
  sprintf(erreur,"OgEmlInit: malloc error on Revar");
  OgErr(ctrl_xml->herr,erreur); return(0);
  }

return((void *)ctrl_xml);
}






PUBLIC(int) OgXmlFlush(handle)
void *handle;
{
struct og_ctrl_xml *ctrl_xml = (struct og_ctrl_xml *)handle;

DPcFree(ctrl_xml);
DONE;
}



