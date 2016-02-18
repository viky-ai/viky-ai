/*
 *  Logging of an HTTP header
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : February 2004
 *  Version 1.0
*/
#include "ogm_sock.h"



STATICF(char *) OgGetHeaderLineName(pr(int));



PUBLIC(int) OgHttpHeaderLog(hh,where)
struct og_http_header *hh;
char *where;
{
char last_modified[128];
struct tm *gmt;
int i;

OgMessageLog(DOgMlogInLog,where,0,"%s",hh->line[DOgHnHttp].value);
OgMessageLog(DOgMlogInLog,where,0,"%d.%d %d",hh->major,hh->minor,hh->status);

for (i=1; i<DOgMaxHhl; i++) {
  if (hh->line[i].value[0]==0) continue;
  //if (DOgHnStartSetCookie <= i && i < DOgHnEndSetCookie) iname=DOgHnStartSetCookie;
  //else if (DOgHnStartSetCookie2 <= i && i < DOgHnEndSetCookie2) iname=DOgHnStartSetCookie2;
  //else iname=i;
  //OgMessageLog(DOgMlogInLog,where,0,"%s: %s",OgUrlHeaderLine[iname].name,hh->line[i].value);
  OgMessageLog(DOgMlogInLog,where,0,"%s: %s",OgGetHeaderLineName(i),hh->line[i].value);
  switch (i) {
    case DOgHnContentLength:
      OgMessageLog(DOgMlogInLog,where,0,"Content-Length: %d",hh->content_length);
      break;
    case DOgHnLastModified:
      gmt = gmtime(&hh->last_modified);
      /** RFC 2616 prefered format: Sun, 06 Nov 1994 08:49:37 GMT **/
      strftime(last_modified,128,"%a, %d %b %Y %H:%M:%S GMT",gmt);
      if (gmt) OgMessageLog(DOgMlogInLog,where,0,"Last-Modified: %s",last_modified);
      else OgMessageLog(DOgMlogInLog,where,0,"Last-Modified: %x GMT",hh->last_modified);
      break;
    }
  }
OgMessageLog(DOgMlogInLog,where,0,"Header Length is %d",hh->header_length);
DONE;
}




STATICF(char *) OgGetHeaderLineName(line)
int line;
{
int i;
if      (DOgHnStartSetCookie  <= line && line < DOgHnEndSetCookie)  line=DOgHnStartSetCookie;
else if (DOgHnStartSetCookie2 <= line && line < DOgHnEndSetCookie2) line=DOgHnStartSetCookie2;

for (i=0; OgUrlHeaderLine[i].iname; i++) {
  if (OgUrlHeaderLine[i].value != line) continue;
  return(OgUrlHeaderLine[i].name);
  }
return("Unknown");
}




