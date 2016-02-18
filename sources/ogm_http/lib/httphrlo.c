/*
 *  Logging an HTTP header
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August 2006
 *  Version 1.0
*/
#include "ogm_http.h"



STATICF(int) OgHttpHeaderLogLine(pr_(struct og_ctrl_http *) pr_(struct og_http_header2 *) pr_(int) pr(unsigned char *));





PUBLIC(char *) OgHttpHeaderRequestMethodString(request_method)
int request_method;
{
switch (request_method) {
  case DOgHttpHeaderTypeOption: return("OPTION");
  case DOgHttpHeaderTypeGet: return("GET");
  case DOgHttpHeaderTypeHead: return("HEAD");
  case DOgHttpHeaderTypePost: return("POST");
  case DOgHttpHeaderTypePut: return("PUT");
  case DOgHttpHeaderTypeDelete: return("DELETE");
  case DOgHttpHeaderTypeTrace: return("TRACE");
  case DOgHttpHeaderTypeConnect: return("CONNECT");
  }
return("UNKNOWN");
}





PUBLIC(char *) OgHttpHeaderLineString(line)
int line;
{
int i;
for (i=0; OgHttpHeaderLine[i].iname; i++) {
  if (OgHttpHeaderLine[i].value != line) continue;
  return(OgHttpHeaderLine[i].name);
  }
return("Unknown");
}





PUBLIC(int) OgHttpHeader2Log(handle,hh)
void *handle; struct og_http_header2 *hh;
{
struct og_ctrl_http *ctrl_http = (struct og_ctrl_http *)handle;
unsigned char out[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
int iout,retour,nstate0,nstate1;
char b[DOgHttpHeaderLineSize];

OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"  %s (raw)",hh->first_line);
if (hh->request_method > 0) {
  sprintf(b,"%s %s HTTP/%d.%d",OgHttpHeaderRequestMethodString(hh->request_method)
    , hh->request_uri, hh->major, hh->minor);
  }
else {
  sprintf(b,"HTTP/%d.%d %d", hh->major, hh->minor, hh->status);
  }
OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"  %s (calculated)",b);

if ((retour=OgAutScanf(hh->ha_header,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    IFE(retour);
    IFE(OgHttpHeaderLogLine(ctrl_http,hh,iout,out));
    }
  while((retour=OgAutScann(hh->ha_header,&iout,out,nstate0,&nstate1,states)));
  }

OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0
  ,"  Header Length is %d",hh->header_length);

DONE;
}




STATICF(int) OgHttpHeaderLogLine(ctrl_http,hh,is,s)
struct og_ctrl_http *ctrl_http;
struct og_http_header2 *hh;
int is; unsigned char *s;
{
char b[DOgHttpHeaderLineSize],last_modified[128];
int i,colon=(-1),attr;
struct tm *gmt;

memcpy(b,s,is); b[is]=0;

for (i=0; i<is; i++) {
  if (b[i]==':') { b[i]=0; colon=i; break; }
  }
if (colon<0) DONE;
attr=atoi(b);
OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0
  ,"  %s: %s",OgHttpHeaderLineString(attr),b+colon+1);

switch (attr) {
  case DOgHttpHeaderLineContentLength:
    OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"  Content-Length: %d (calculated)",hh->content_length);
    break;
  case DOgHttpHeaderLineLastModified:
    gmt = gmtime(&hh->last_modified);
    /** RFC 2616 prefered format: Sun, 06 Nov 1994 08:49:37 GMT **/
    strftime(last_modified,128,"%a, %d %b %Y %H:%M:%S GMT",gmt);
    if (gmt) OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"  Last-Modified: %s  (calculated)",last_modified);
    else OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"  Last-Modified: %x GMT  (calculated)",hh->last_modified);
    break;
  }

DONE;
}


