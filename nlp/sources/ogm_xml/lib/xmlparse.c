/*
 *  Generic function for parsing XML.
 *  Copyright (c) 2003-2006 Pertimm by Patrick Constant
 *  Dev : April 2003, March,October 2004, October 2006
 *  Version 1.3
*/
#include "ogm_xml.h"






PUBLIC(int) OgParseXml(herr,is_file,ib,b,func,ptr)
void *herr; int is_file; int ib; unsigned char *b;
int (*func)(pr_(void *) pr_(int) pr_(int) pr(unsigned char *));
void *ptr;
{
char erreur[DOgErrorSize];
int i,c,state=1,end=0,start=0,retour=0;

if (is_file) {
  sprintf(erreur,"OgParseXml: is_file option not done yet");
  OgErr(herr,erreur); DPcErr;
  }

for (i=0; !end; i++) {
  if (i<ib) c = b[i];
  else { end=1; c= ' '; }
  switch (state) {
    /** before any tag **/
    case 1:
      if (c=='<') {
        start=i+1; state=2;
        }
      break;
    /** in tag **/
    case 2:
      if (c=='>') {
        IFE(retour=func(ptr,DOgParseXmlTag,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+1;
        state=3;
        }
      break;
    /** between tags **/
    case 3:
      if (c=='<') {
        IFE(retour=func(ptr,DOgParseXmlContent,i-start,b+start));
        if (retour) goto endOgParseXml;
        if (i+4<=ib && !memcmp(b+i,"<!--",4)) {
          start=i+4; state=4; i+=3;
          }
        else if (i+9<=ib && !Ogmemicmp(b+i,"<![cdata[",9)) {
          start=i+9; state=5; i+=8;
          }
        else {
          start=i+1;
          state=2;
          }
        }
      break;
    /** in comments **/
    case 4:
      if (c=='-' && i+3<=ib && !memcmp(b+i,"-->",3)) {
        IFE(retour=func(ptr,DOgParseXmlComment,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+3;
        state=3;
        i+=2;
        }
      break;
    /** in cdata **/
    case 5:
      if (c==']' && i+3<=ib && !memcmp(b+i,"]]>",3)) {
        IFE(retour=func(ptr,DOgParseXmlCdata,i-start,b+start));
        if (retour) goto endOgParseXml;
        start=i+3;
        state=3;
        i+=2;
        }
      break;
    }
  }

endOgParseXml:

return retour;
}


