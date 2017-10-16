/*
 *	Working on http header.
 *	Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *	Dev : August 2006, October 2007
 *	Version 1.1
*/
#include "ogm_http.h"



/*
 *  We accept only 2 "\r\n" as header termination string, 
*/

PUBLIC(int) OgHttpHeaderLength(handle,is,s,pheader_length)
void *handle; int is; unsigned char *s;
int *pheader_length;
{
int i,header_length=0,state=1;

for (i=0; i<is; i++) {
  switch (state) {
    /** before first "\r\n" **/
    case 1:
      if (i+2<=is && !memcmp(s+i,"\r\n",2)) { i+=1; state=2; }
      else state=1;
      break;
    /** after first "\r\n" **/
    case 2:
      /** October 18th 2007: we must accept only only 2 "\r\n" **/
      if (i+2<=is && !memcmp(s+i,"\r\n",2)) { i+=1; header_length=i+1; goto endOgHttpHeaderLength; }
      else state=1;
      break;
    }
  }
endOgHttpHeaderLength:

*pheader_length=header_length;
if (header_length>0) return(1);
return(0);
}



