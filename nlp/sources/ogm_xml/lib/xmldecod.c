/*
 *  Xml encoding an XML content.
 *  Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *  Dev: July 2006, July 2007
 *  Version 1.1
*/
#include "ogm_xml.h"


/*
 *  Making a content XML-compatible. codepage is not used as of now,
 *  but will be used to encode a Unicode buffer when necessary.
*/

PUBLIC(int) OgXmlDecode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout, int codepage, int *position, int *truncated)
{
int i,j,c,ientity=0,centity=0;

if (truncated) *truncated=0;

for (i=j=0; i<ibin; i++) {
  c=bin[i]; centity=0;
  if (c=='&') {
    if      (i+4<=ibin && !Ogmemicmp(bin+i,"&lt;",4))   { ientity=4; centity='<'; }
    else if (i+4<=ibin && !Ogmemicmp(bin+i,"&gt;",4))   { ientity=4; centity='>'; }
    else if (i+5<=ibin && !Ogmemicmp(bin+i,"&amp;",5))  { ientity=5; centity='&'; }
    else if (i+6<=ibin && !Ogmemicmp(bin+i,"&apos;",6)) { ientity=6; centity='\''; }
    else if (i+6<=ibin && !Ogmemicmp(bin+i,"&quot;",6)) { ientity=6; centity='"'; }
    }
  if (j+1>=sbout) {
    if (truncated) *truncated=1;
    break;
    }
  if (centity) {
    if (position) position[j]=i;
    bout[j++]=centity;
    i+=ientity-1;
    }
  else {
    if (position) position[j]=i;
    bout[j++]=c;
    }
  }

*ibout=j;
if (position) position[j]=i;
bout[j]=0;
DONE;
}



