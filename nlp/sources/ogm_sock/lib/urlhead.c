/*
 *  Parsing of an HTTP header
 *  Copyright (c) 2001-2004 Pertimm by Patrick Constant
 *  Dev : July 2001, February 2004
 *  Version 1.1
*/
#include "ogm_sock.h"
#include "urlhead.h"


STATICF(int) OgHttpHeaderParse1(pr_(int) pr_(char *) pr(struct og_http_header *));
STATICF(int) OgGetHttp(pr_(char *) pr(struct og_http_header *));
STATICF(int) OgGetContentLength(pr_(char *) pr(size_t *));
STATICF(int) GetMonthNumber(pr_(int) pr(char *));



/*
 * Parsing a URL header. We consider that the header lines can be in any order
 * and that there is no difference between upper and lower cases in the fields.
 * We also consider that a CR is always the end of the header line.
 * examples of an HTTP header at end of file.
*/

PUBLIC(int) OgHttpHeaderParse(iheader,header,hh)
int iheader; char *header; struct og_http_header *hh;
{
char erreur[DPcSzErr];
int i,c,n,start=0,state=1,eheader=0;

memset(hh,0,sizeof(struct og_http_header));

/** looking for end of header **/
for (i=n=0; i<iheader; i++) {
  if (header[i]=='\r') continue;
  if (header[i]=='\n') { n++; if (n>=2) { eheader=i+1; break; } }
  else n=0;
  }

if (eheader==0) {
  sprintf(erreur,"OgHttpHeaderParse: header should finish with \\r\\n\\r\\n");
  PcErr(-1,erreur);  DPcErr;
  }

for (i=0; i<eheader; i++) {
  c=header[i];
  switch (state) {
    case 1: 
      if (c=='\r' || c=='\n');
      else { start=i; state=2; }
      break;
    case 2: 
      if (c=='\r' || c=='\n') {
        IFE(OgHttpHeaderParse1(i-start,header+start,hh));
        state=1;
        }
      break;
    }
  }

hh->header_length = eheader;

DONE;
}


STATICF(int) OgHttpHeaderParse1(ih,h,hh)
int ih; char *h; struct og_http_header *hh;
{
int i,is,iv,iname=0,line=-1,offset=0; 
char s[DPcPathSize],v[DPcPathSize];
char erreur[DOgErrorSize];

if (ih >= DPcPathSize-1) {
  /* June 24th 2006: this can happen for example on site http://www.cmykpages.com, 
   * there is a very big Location attribute, we could cut it, but, for the moment
   * we keep this as an error because it is rare */
  sprintf(erreur,"OgHttpHeaderParse1: ih (%d)>= DPcPathSize-1 (%d) for '%.200s'", ih, DPcPathSize-1, h);
  PcErr(-1,erreur); DPcErr;
  }

memcpy(s,h,ih); s[ih]=0;
IFE(OgTrimString(s,s)); is=strlen(s);

for (i=0; OgUrlHeaderLine[i].iname; i++) {
  iname = OgUrlHeaderLine[i].iname;
  if (iname >= DOgMaxHhl) {
    sprintf(erreur,"OgHttpHeaderParse1: iname (%d)>= DOgMaxHhl (%d)", iname, DOgMaxHhl);
    PcErr(-1,erreur); DPcErr;
    }
  if (is < iname) continue;
  if (Ogmemicmp(s,OgUrlHeaderLine[i].name,iname)) continue;
  line=OgUrlHeaderLine[i].value;
  if (line==DOgHnHttp || PcIsspace(s[iname]) || s[iname] == ':') break;
  }

if (line<0) DONE;

if (line == DOgHnHttp) sprintf(v,"%.*s",DOgMhhls-1,s);
else {
  for (i=iname; i<is; i++) {
    if (isspace(s[i])) continue;
    if (s[i] == ':') continue;
    break;
    }
  sprintf(v,"%.*s",DOgMhhls-1,s+i); 
  }

offset=0;
switch (line) {
  case DOgHnStartSetCookie:  
    offset=hh->nbSetCookie;  
    if (offset >= DOgHnEndSetCookie-DOgHnStartSetCookie) DONE;
    hh->nbSetCookie++; break;
  case DOgHnStartSetCookie2: 
    offset=hh->nbSetCookie2; 
    if (offset >=DOgHnEndSetCookie-DOgHnStartSetCookie) DONE;
    hh->nbSetCookie2++; break;
  }

IFE(OgTrimString(v,v)); iv=strlen(v);
strcpy(hh->line[line+offset].value,v);
hh->line[line+offset].ivalue=iv;

switch (line) {
  case DOgHnHttp: IFE(OgGetHttp(v,hh)); break;
  case DOgHnContentLength: IFE(OgGetContentLength(v,&hh->content_length)); break;
  case DOgHnLastModified: IFE(OgHttpHeaderGetLastModified(v,&hh->last_modified)); break;
  }

DONE;
}


/* 
 * Working on: HTTP/1.1 200 OK
*/
STATICF(int) OgGetHttp(v,hh)
char *v; struct og_http_header *hh;
{
int ibuf=0; char buf[DOgMhhls]; 
int i,c,state=1,end=0; 

for (i=0; !end; i++) {
  if (v[i]==0) end=0;
  c=PcTolower(v[i]);
  switch (state) {
    /** HTTP/1.1 200 OK **/
    case 1: if (c=='h') state=2; else goto endOgGetHttp; break;
    case 2: if (c=='t') state=3; else goto endOgGetHttp; break;
    case 3: if (c=='t') state=4; else goto endOgGetHttp; break;
    case 4: if (c=='p') state=5; else goto endOgGetHttp; break;
    case 5: if (c=='/') state=6; else goto endOgGetHttp; break;
    case 6:
      if (PcIsdigit(c)) { ibuf=0; buf[ibuf++]=c; state=7; }
      else goto endOgGetHttp;
      break;
    case 7:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (c=='.') { buf[ibuf]=0; hh->major=atoi(buf); ibuf=0; state=8; }
      else goto endOgGetHttp;
      break;
    case 8:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (PcIsspace(c)) { buf[ibuf]=0; hh->minor=atoi(buf); state=9; }
      else goto endOgGetHttp;
      break;
    
    /** 200 OK **/
    case 9:
      if (PcIsdigit(c)) { ibuf=0; buf[ibuf++]=c; state=10; }
      else if (PcIsspace(c)) state=9;
      else goto endOgGetHttp;
      break;
    case 10:
      if (PcIsdigit(c)) buf[ibuf++]=c; 
      else if (PcIsspace(c)) { buf[ibuf]=0; hh->status=atoi(buf); goto endOgGetHttp; }
      else goto endOgGetHttp;
      break;
    }
  }
endOgGetHttp:
DONE;
}


/* 
 * Working on: 7780
*/
STATICF(int) OgGetContentLength(v,content_length)
char *v; size_t *content_length;
{
*content_length=atoi(v);
DONE;
}





PUBLIC(int) OgHttpHeaderGetLastModified(v,last_modified)
char *v; time_t *last_modified;
{
return(OgGetStringDate(v,last_modified));
}



/*
examples of an HTTP header:

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

