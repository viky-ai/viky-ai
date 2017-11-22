/*
 *	Generic function for parsing XML - Unicode version.
 *	Copyright (c) 2003-2006 Pertimm by Patrick Constant
 *	Dev : April 2003, March,October 2004, October 2006
 *  Dev : April 2008
 *	Version 1.4
*/
#include "ogm_xml.h"






PUBLIC(int) OgParseXmlUni(herr,is_file,ib,b,func,ptr)
void *herr; int is_file; int ib; unsigned char *b;
int (*func)(pr_(void *) pr_(int) pr_(int) pr(unsigned char *));
void *ptr;
{
char erreur[DOgErrorSize];
int i,c,state=1,end=0,start=0,retour;

if (is_file) {
  sprintf(erreur,"OgParseXml: is_file option not done yet");
  OgErr(herr,erreur); DPcErr;
  }

for (i=0; !end; i+=2) {
  if (i<ib) c = (b[i]<<8) + b[i+1];
  else { end=1; c= ' '; }
  switch (state) {
    /** before any tag **/
    case 1:
      if (c=='<') {
        start=i+2; state=2;
        }
      break;
    /** in tag **/
    case 2:
      if (c=='>') {
        IFE(retour=func(ptr,DOgParseXmlTag,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+2;
        state=3;
        }
      break;
    /** between tags **/
    case 3:
      if (c=='<') {
        IFE(retour=func(ptr,DOgParseXmlContent,i-start,b+start));
        if (retour) goto endOgParseXml;
        if (i+8<=ib && !memcmp(b+i,"\0<\0!\0-\0-",8)) {
          start=i+8; state=4; i+=6;
          }
        else if (i+18<=ib && !Ogmemicmp(b+i,"\0<\0!\0[\0c\0d\0a\0t\0a\0[",18)) {
          start=i+18; state=5; i+=16;
          }
        else {
          start=i+2;
          state=2;
          }
        }
      break;
    /** in comments **/
    case 4:
      if (c=='-' && i+6<=ib && !memcmp(b+i,"\0-\0-\0>",6)) {
        IFE(retour=func(ptr,DOgParseXmlComment,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+6;
        state=3;
        i+=4;
        }
      break;
    /** in cdata **/
    case 5:
      if (c==']' && i+6<=ib && !memcmp(b+i,"\0]\0]\0>",6)) {
        IFE(retour=func(ptr,DOgParseXmlCdata,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+6;
        state=3;
        i+=4;
        }
      break;
    }
  }

endOgParseXml:

DONE;
}


