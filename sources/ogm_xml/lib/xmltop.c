/*
 *	Getting the top tag of an XML file
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : September 2006
 *	Version 1.0
*/
#include "ogm_xml.h"



/*
 * Gets the top tag of an XML file. the file should start with:
 * <?xml version="1.0" encoding="ISO-8859-1"?>
 * <top_tag>
*/

PUBLIC(int) OgXmlTopTag(herr,is,s,top_tag_size,top_tag)
void *herr; int is; unsigned char *s;
int top_tag_size; unsigned char *top_tag;
{
int i,state=1,start=0,length,found=0;

top_tag[0]=0;

for (i=0; i<is; i++) {
  switch(state) {
    /** before <? **/
    case 1:
      if (i+1<=is && !memcmp(s+i,"<?",2)) state=2;
      break;
    /** before ?> **/
    case 2:
      if (i+1<=is && !memcmp(s+i,"?>",2)) state=3;
      break;
    /** before <top_tag **/
    case 3:
      /** avoids such tags as "<!DOCTYPE3 or "<!ENTITY" **/
      if (s[i]=='<' && i+1<=is && s[i+1]!='!') { start=i+1; state=4; }
      break;
    /** before top_tag> **/
    case 4:
      if (s[i]=='>') { 
        length=i-start; 
        if (length > top_tag_size) length=top_tag_size;
        memcpy(top_tag,s+start,length); top_tag[length]=0;
        OgTrimString(top_tag,top_tag); found=1;
        goto endOgXmlTopTag; 
        }
      break;
    }
  }

endOgXmlTopTag:

return(found);
}




