/*
 *	Getting encoding information from the XML file.
 *	Copyright (c) 2005 Pertimm by Patrick Constant
 *	Dev : May 2005
 *	Version 1.0
*/
#include "ogm_xml.h"


STATICF(int) OgXmlInformationTag(pr_(void *) pr(struct og_read_tag *));



/*
 * The Unicode byte order marker (BOM) is 0xFE 0xFF, so, if we find
 * those chars in this order, we know that the encoding 
 * is in the same order. To get the encoding, we typically
 * look at the first line of the file:
 * <?xml version="1.0" encoding="ISO-8859-1"?>
*/

PUBLIC(int) OgXmlInformation(herr,ib,b,info)
void *herr; int ib; unsigned char *b;
struct og_xml_information *info;
{
char erreur[DOgErrorSize];
int i,state=1,start_tag=0,length_tag=0;

memset(info,0,sizeof(struct og_xml_information));

if (ib>=2 && b[0]==0xfe && b[1]==0xff) {
  info->encoding=DOgCodePageUnicode;
  info->unicode_same_order=1;
  }
else if (ib>=2 && b[0]==0xff && b[1]==0xfe) {
  info->encoding=DOgCodePageUnicode;
  info->unicode_same_order=0;
  }
/** This is a guess **/
else if (ib>=2 && b[0]==0) {
  info->encoding=DOgCodePageUnicode;
  info->unicode_same_order=1;
  }
/** This is a guess **/
else if (ib>=2 && b[1]==0) {
  info->encoding=DOgCodePageUnicode;
  info->unicode_same_order=0;
  }
if (info->encoding==DOgCodePageUnicode) {
  sprintf(erreur,"OgXmlInformation: Unicode encoding not done yet");
  OgErr(herr,erreur); DPcErr;
  }

for (i=0; i<ib; i++) {
  if (state==1) {
    if (b[i]=='<') { start_tag=i+1; state=2; }
    } 
  else if (state==2) {
    if (b[i]=='>') { length_tag=i-start_tag; break; }
    }
  }

if (length_tag > 0) {
  IFE(OgReadTag(length_tag,b+start_tag,0,OgXmlInformationTag,info));
  }

DONE;
}





STATICF(int) OgXmlInformationTag(ptr,rt)
void *ptr; struct og_read_tag *rt;
{
struct og_xml_information *info = (struct og_xml_information *)ptr;
int ivalue;

if (rt->closing_tag) DONE;

if (Ogstricmp("?xml",rt->name)) DONE;
if (Ogstricmp("encoding",rt->attr)) DONE;

ivalue=strlen(rt->value);
if (ivalue>=DOgMaxEncoding) DONE;

strcpy(info->sencoding,rt->value);
OgTrimString(info->sencoding,info->sencoding);

info->encoding=OgCharsetToCode(info->sencoding);

DONE;
}





/*
 *  Checks if the buffer/file is an XML file by looking 
 *  for string "<?xml", We suppose that we know that the 
 *  file is unicode or not and use valid unicode char FEFF
 *  (FFFE is not valid) to see which endian is the unicode.
 *  If the FEFF marker is not there we assume FEFF order.
*/
#define DOgIsXmlBufferSize   64

PUBLIC(int) OgIsXml(herr,is_file,is_unicode,ibuffer,buffer)
void *herr; int is_file,is_unicode;
int ibuffer; unsigned char *buffer;
{
int one=(is_unicode?2:1);
int ibuf=DOgIsXmlBufferSize*one;
unsigned char buf[DOgIsXmlBufferSize*2+2+8];
int ib=0; unsigned short b[DOgIsXmlBufferSize+1];
short xml[5]={'<','?','x','m','l'}; int ixml=5*one;
int i,is; unsigned char *s;
char erreur[DPcPathSize];
struct stat filestat;
FILE *fd;

if (is_file) {
  int ifilename; unsigned char filename[DPcPathSize];
  ifilename=ibuffer; if (ifilename>=DPcPathSize-1) ifilename=DPcPathSize-1;
  memcpy(filename,buffer,ifilename); filename[ifilename]=0;
  IFn(fd=fopen(filename,"rb")) {
    sprintf(erreur,"OgIsXml: impossible to fopen '%s'",filename);
    OgErr(herr,erreur); DPcErr;
    }
  IFx(fstat(fileno(fd),&filestat)) {
    sprintf(erreur,"OgIsXml: impossible to fstat '%s'",filename);
    fclose(fd); OgErr(herr,erreur); DPcErr;
    }
  if (ibuf>filestat.st_size) ibuf=filestat.st_size;
  IFn(fread(buf,1,ibuf,fd)) {
    sprintf(erreur,"OgIsXml: impossible to fread '%s'",filename);
    fclose(fd); OgErr(herr,erreur); DPcErr;
    }
  fclose(fd);
  is=ibuf; s=buf;
  }
else {
  is=ibuffer; s=buffer;
  }

if (is<ixml) return(0);

if (is_unicode) {
  int zero=0,un=1;
  if (s[0]==0xff && s[1]==0xfe) { zero=1; un=0; }
  for (i=0; i<is; i+=2) {
    b[ib++]=(s[i+zero]<<8) && s[i+un];
    if (ib>=DOgIsXmlBufferSize-1) break;
    }

  }
else {
  for (i=0; i<is; i++) {
    b[ib++]=s[i];
    if (ib>=DOgIsXmlBufferSize-1) break;
    }
  }

for (i=0; i<ib; i++) b[i]=PcTolower(b[i]);
for (i=0; i+5<ib; i++) {
  if (!memcmp(b+i,xml,sizeof(short)*5)) return(1);
  }


return(0);
}


