/*
 *	Working on http hh.
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : August 2006
 *	Version 1.0
*/
#include "ogm_http.h"



STATICF(int) HeaderRead1(pr_(struct og_ctrl_http *) pr_(int) pr_(unsigned char *) pr(struct og_http_header2 *));
STATICF(int) HeaderReadFirstLine(pr_(struct og_ctrl_http *) pr_(int) pr_(unsigned char *) pr(struct og_http_header2 *));
STATICF(int) ReadResponse(pr_(struct og_ctrl_http *) pr_(char *) pr(struct og_http_header2 *));
STATICF(int) ReadRequest(pr_(struct og_ctrl_http *) pr_(char *) pr(struct og_http_header2 *));




/*
 *  Reads the hh into an automaton and returns 1 if found and zero otherwise.
*/

PUBLIC(int) OgHttpHeaderRead(handle,is,s,hh)
void *handle; int is; unsigned char *s;
struct og_http_header2 *hh;
{
struct og_ctrl_http *ctrl_http = (struct og_ctrl_http *)handle;
int i,c,found_header,header_length,start=0,state=1;

IFE(OgAutReset(ctrl_http->ha_header));
memset(hh,0,sizeof(struct og_http_header2));
hh->ha_header=ctrl_http->ha_header;

IFE(found_header=OgHttpHeaderLength(handle,is,s,&header_length));

if (!found_header) return(0);

hh->header_length=header_length;

for (i=0; i<header_length; i++) {
  c=s[i];
  switch (state) {
    case 1: 
      if (c=='\r' || c=='\n');
      else { start=i; state=2; }
      break;
    case 2: 
      if (c=='\r' || c=='\n') {
        IFE(HeaderRead1(ctrl_http,i-start,s+start,hh));
        state=1;
        }
      break;
    }
  }

return(1);
}





STATICF(int) HeaderRead1(ctrl_http,ih,h,hh)
struct og_ctrl_http *ctrl_http; int ih; unsigned char *h;
struct og_http_header2 *hh;
{
char s[DOgHttpHeaderLineSize],v[DOgHttpHeaderLineSize],b[DOgHttpHeaderLineSize+10];
int i,is,iv,iname=0,attr=(-1);
char erreur[DOgErrorSize];

if (ih >= DOgHttpHeaderLineSize-1) {
  /* June 24th 2006: this can happen for example on site http://www.cmykpages.com, 
   * there is a very big Location attribute, we could cut it, but, for the moment
   * we keep this as an error because it is rare */
  sprintf(erreur,"HeaderRead1: ih (%d)>= DOgHttpHeaderLineSize-1 (%d) for '%.200s'"
    , ih, DOgHttpHeaderLineSize-1, h);
  OgErr(ctrl_http->herr,erreur); DPcErr;
  }

memcpy(s,h,ih); s[ih]=0;
IFE(OgTrimString(s,s)); is=strlen(s);

for (i=0; OgHttpHeaderLine[i].iname; i++) {
  iname = OgHttpHeaderLine[i].iname;
  if (is < iname) continue;
  if (Ogmemicmp(s,OgHttpHeaderLine[i].name,iname)) continue;
  attr=OgHttpHeaderLine[i].value;
  if (PcIsspace(s[iname]) || s[iname] == ':') break;
  }

if (attr<0) {
  /** Handling the first line **/
  IFE(HeaderReadFirstLine(ctrl_http,is,s,hh));
  DONE;
  }

for (i=iname; i<is; i++) {
  if (isspace(s[i])) continue;
  if (s[i] == ':') continue;
  break;
  }
strcpy(v,s+i); 
IFE(OgTrimString(v,v)); iv=strlen(v);

sprintf(b,"%d:%s",attr,v);
IFE(OgAutAdd(ctrl_http->ha_header,strlen(b),b));

switch (attr) {
  case DOgHttpHeaderLineContentLength: hh->content_length=atoi(v); break;
  case DOgHttpHeaderLineLastModified: IFE(OgGetStringDate(v,&hh->last_modified));  break;
  }

DONE;
}




/*
 *  First line of HTTP protocol is typically:
 *    POST /ssrv/control HTTP/1.0
 *    HTTP/1.1 200 OK
*/


STATICF(int) HeaderReadFirstLine(ctrl_http,ih,h,hh)
struct og_ctrl_http *ctrl_http; int ih; unsigned char *h;
struct og_http_header2 *hh;
{
int i,attr=0,iname=(-1);

for (i=0; OgHttpHeaderType[i].iname; i++) {
  iname = OgHttpHeaderType[i].iname;
  if (ih < iname) continue;
  if (Ogmemicmp(h,OgHttpHeaderType[i].name,iname)) continue;
  attr=OgHttpHeaderType[i].value;
  if (PcIsspace(h[iname]) || h[iname] == '/') break;
  }

if (iname<0) DONE;

memcpy(hh->first_line,h,ih); hh->first_line[ih]=0;

switch(attr) {
  case DOgHttpHeaderTypeOption:
  case DOgHttpHeaderTypeGet:
  case DOgHttpHeaderTypeHead:
  case DOgHttpHeaderTypePost:
  case DOgHttpHeaderTypePut:
  case DOgHttpHeaderTypeDelete:
  case DOgHttpHeaderTypeTrace:
  case DOgHttpHeaderTypeConnect:
    IFE(ReadRequest(ctrl_http,h+iname,hh));
    hh->request_method=attr;
    break;
  case DOgHttpHeaderTypeHttp:
    IFE(ReadResponse(ctrl_http,h,hh));
    break;
  }

DONE;
}




/* 
 * Working on: HTTP/1.1 200 OK
*/
STATICF(int) ReadResponse(ctrl_http,v,hh)
struct og_ctrl_http *ctrl_http;
char *v; struct og_http_header2 *hh;
{
int ibuf=0; char buf[DOgHttpHeaderLineSize]; 
int i,c,state=1,end=0; 

for (i=0; !end; i++) {
  if (v[i]==0) end=1;
  c=PcTolower(v[i]);
  switch (state) {
    /** HTTP/1.1 200 OK **/
    case 1: if (c=='h') state=2; else goto endReadResponse; break;
    case 2: if (c=='t') state=3; else goto endReadResponse; break;
    case 3: if (c=='t') state=4; else goto endReadResponse; break;
    case 4: if (c=='p') state=5; else goto endReadResponse; break;
    case 5: if (c=='/') state=6; else goto endReadResponse; break;
    case 6:
      if (PcIsdigit(c)) { ibuf=0; buf[ibuf++]=c; state=7; }
      else goto endReadResponse;
      break;
    case 7:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (c=='.') { buf[ibuf]=0; hh->major=atoi(buf); ibuf=0; state=8; }
      else goto endReadResponse;
      break;
    case 8:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (PcIsspace(c)) { buf[ibuf]=0; hh->minor=atoi(buf); state=9; }
      else goto endReadResponse;
      break;
    
    /** 200 OK **/
    case 9:
      if (PcIsdigit(c)) { ibuf=0; buf[ibuf++]=c; state=10; }
      else if (PcIsspace(c)) state=9;
      else goto endReadResponse;
      break;
    case 10:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (PcIsspace(c)) { buf[ibuf]=0; hh->status=atoi(buf); goto endReadResponse; }
      else goto endReadResponse;
      break;
    }
  }
endReadResponse:
DONE;
}




/* 
 * Working on: <request> URI HTTP/1.1
 * per RFC 2616: Request-Line = Method SP Request-URI SP HTTP-Version CRLF
*/

STATICF(int) ReadRequest(ctrl_http,v,hh)
struct og_ctrl_http *ctrl_http;
char *v; struct og_http_header2 *hh;
{
int ibuf=0; char buf[DOgHttpHeaderLineSize]; 
int i,c,state=1,end=0,start=1,length; 

for (i=0; !end; i++) {
  if (v[i]==0) { c=' '; end=1; }
  else c=PcTolower(v[i]);
  switch (state) {
    case 1: 
      if (!PcIsspace(c)) { start=i; state=2; }
      break;
    case 2: 
      if (PcIsspace(c)) {
        length=i-start;
        if (length>DPcPathSize-1) length=DPcPathSize-1;
        memcpy(hh->request_uri,v+start,length); hh->request_uri[length]=0;
        state=3; 
        }
      break;
    /** HTTP/1.1 200 OK **/
    case 3: if (c=='h') state=4; else goto endReadRequest; break;
    case 4: if (c=='t') state=5; else goto endReadRequest; break;
    case 5: if (c=='t') state=6; else goto endReadRequest; break;
    case 6: if (c=='p') state=7; else goto endReadRequest; break;
    case 7: if (c=='/') state=8; else goto endReadRequest; break;
    case 8:
      if (PcIsdigit(c)) { ibuf=0; buf[ibuf++]=c; state=9; }
      else goto endReadRequest;
      break;
    case 9:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (c=='.') { buf[ibuf]=0; hh->major=atoi(buf); ibuf=0; state=10; }
      else goto endReadRequest;
      break;
    case 10:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (PcIsspace(c)) { buf[ibuf]=0; hh->minor=atoi(buf); goto endReadRequest; }
      else goto endReadRequest;
      break;
    
    }
  }
endReadRequest:
DONE;
}



/*
examples of a response HTTP header:

HTTP/1.1 200 OK
Date: Sun, 29 Jul 2001 08:25:52 GMT
Server: Apache/1.3.19 (Win32)
Last-Modified: Wed, 27 Sep 2000 20:27:12 GMT
ETag: "0-254-39d25820"
Accept-Ranges: bytes
Content-Length: 596
Connection: close
Content-Type: text/html

HTTP/1.1 200 OK
Server: Microsoft-IIS/4.0
Date: Tue, 07 Aug 2001 18:56:33 GMT
Content-Type: text/html
Accept-Ranges: bytes
Last-Modified: Mon, 16 Jul 2001 11:37:58 GMT
ETag: "b66a2dc3ebdc11:2960"
Content-Length: 7780
*/

