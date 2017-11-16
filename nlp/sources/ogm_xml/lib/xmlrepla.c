/*
 *  Generic replacement function using an XML replacement template file.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : July 2005
 *  Version 1.0
*/
#include "ogm_xml.h"


#define DEVERMINE

/** Value for xml tag and xdata type **/
#define DOgReplaXmlTagNil            0
#define DOgReplaXmlTagReplacement    1
#define DOgReplaXmlTagFile           2
#define DOgReplaXmlTagFrom           3
#define DOgReplaXmlTagTo             4


#define DOgMaxXmlPath       10

struct og_replace_info {
  struct og_ctrl_xml *ctrl_xml;
  int ixml_path, xml_path[DOgMaxXmlPath];
  int iB; unsigned char *B;
  int end_tag;
  int start_file,length_file;
  int start_from,length_from;
  int start_to,length_to;
  };

struct og_men_xml_tag {
  int value, iname; char *name;
  };

struct og_men_xml_tag OgReplaXmlTag[] =  {
  { DOgReplaXmlTagReplacement, 11, "replacement" },
  { DOgReplaXmlTagFile,         4, "file" },
  { DOgReplaXmlTagFrom,         4, "from" },
  { DOgReplaXmlTagTo,           2, "to" },
  { 0, 0, "" }
  };



STATICF(int) OgXmlReplace1(pr_(void *) pr_(int) pr_(int) pr(unsigned char *));
STATICF(int) OgXmlReplace2(pr(struct og_replace_info *));
STATICF(int) OgXmlReplace3(pr_(struct og_ctrl_xml *) pr_(int)
                           pr_(unsigned char *) pr(unsigned char *));
STATICF(int) OgXmlReplace4(pr_(struct og_ctrl_xml *) pr_(unsigned char *)
                           pr_(unsigned char *) pr(unsigned char *));


PUBLIC(int) OgXmlReplaceAdd(handle,name,value)
void *handle; unsigned char *name, *value;
{
struct og_ctrl_xml *ctrl_xml = (struct og_ctrl_xml *)handle;
IFE(AddRevar(ctrl_xml,name,value));
DONE;
}




PUBLIC(int) OgXmlReplace(handle,xml_file)
void *handle; char *xml_file;
{
struct og_ctrl_xml *ctrl_xml = (struct og_ctrl_xml *)handle;
struct og_replace_info cinfo, *info=&cinfo;
int iB; unsigned char *B;
struct stat filestat;
FILE *fd;

IF(stat(xml_file,&filestat)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace: impossible to stat '%s', skipping file",xml_file);
  DONE;
  }

if (ctrl_xml->loginfo->trace & DOgXmlTraceReplace) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace starting on '%s' size=%d",xml_file,filestat.st_size);
  }

IFn(B=(char *)malloc(filestat.st_size+9)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace: impossible to allocate %d bytes for '%s', skipping",filestat.st_size,xml_file);
  DONE;
  }

IFn(fd=fopen(xml_file,"r")) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace: impossible to fopen '%s', skipping",xml_file);
  DPcFree(B); DONE;
  }

IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace: impossible to fread '%s', skipping",xml_file);
  fclose(fd); DPcFree(B); DONE;
  }

fclose(fd);


memset(info,0,sizeof(struct og_replace_info));
info->ctrl_xml=ctrl_xml;
info->iB=iB; info->B=B;

IFE(OgParseXml(ctrl_xml->herr,0,iB,B,OgXmlReplace1,(void *)info));

DPcFree(B);

DONE;
}





STATICF(int) OgXmlReplace1(ptr,type,ib,b)
void *ptr; int type; int ib; unsigned char *b;
{
struct og_replace_info *info = (struct og_replace_info *)ptr;
struct og_ctrl_xml *ctrl_xml = (struct og_ctrl_xml *)info->ctrl_xml;
int i,value,iname_tag;

#ifdef DEVERMINE
if (ctrl_xml->loginfo->trace & DOgXmlTraceReplace) {
  char buffer[DPcPathSize];
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0,"OgXmlReplace1: %d '%.*s'", type, ib, b);
  buffer[0]=0;
  for (i=0; i<info->ixml_path; i++) {
    sprintf(buffer+strlen(buffer),"%d ",info->xml_path[i]);
    }
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0,"OgXmlReplace1: xml_path: %s", buffer);
  }
#endif

if (type==DOgParseXmlTag) {
  value=DOgReplaXmlTagNil;
  for (i=0; i<ib; i++) b[i]=tolower(b[i]);
  if (b[0]=='/' || b[ib-1]=='/') info->end_tag=1; else info->end_tag=0;
  for (i=0; i<ib; i++) { if (PcIsspace(b[i])) break;} iname_tag=i;
  for (i=0; OgReplaXmlTag[i].value; i++) {
    if (OgReplaXmlTag[i].iname==iname_tag-info->end_tag && !memcmp(OgReplaXmlTag[i].name,b+info->end_tag,iname_tag-info->end_tag)) {
      value=OgReplaXmlTag[i].value; break;
      }
    }
  if (value!=DOgReplaXmlTagNil) {
    if (info->end_tag) { if (info->ixml_path > 0) info->ixml_path--; }
    else { if (info->ixml_path < DOgMaxXmlPath-1) info->xml_path[info->ixml_path++]=value; }
    if (value==DOgReplaXmlTagReplacement) {
      if (info->end_tag) {
        /** Here we handle the replacement operation **/
        IFE(OgXmlReplace2(info));
        }
      else {
        info->start_file=0; info->length_file=0;
        info->start_from=0; info->length_from=0;
        info->start_to=0; info->length_to=0;
        }
      }
    }
  }
else if (type==DOgParseXmlContent) {
  if (info->xml_path[info->ixml_path-2]==DOgReplaXmlTagReplacement) {
    if (info->xml_path[info->ixml_path-1]==DOgReplaXmlTagFile) {
      info->start_file=b-info->B; info->length_file=ib;
      }
    else if (info->xml_path[info->ixml_path-1]==DOgReplaXmlTagFrom) {
      info->start_from=b-info->B; info->length_from=ib;
      }
    else if (info->xml_path[info->ixml_path-1]==DOgReplaXmlTagTo) {
      info->start_to=b-info->B; info->length_to=ib;
      }
    }
  }

DONE;
}






STATICF(int) OgXmlReplace2(info)
struct og_replace_info *info;
{
struct og_ctrl_xml *ctrl_xml = (struct og_ctrl_xml *)info->ctrl_xml;
unsigned char file[DPcPathSize], from[DOgMaxRevarSize], to[DOgMaxRevarSize];
unsigned char tor[DOgMaxRevarSize*2];
int ifile,ifrom,ito;

ifile=info->length_file;
if (ifile>DPcPathSize-1) ifile=DPcPathSize-1;
memcpy(file,info->B+info->start_file,ifile); file[ifile]=0;

ifrom=info->length_from;
if (ifrom>DOgMaxRevarSize-1) ifrom=DOgMaxRevarSize-1;
memcpy(from,info->B+info->start_from,ifrom); from[ifrom]=0;

ito=info->length_to;
if (ito>DOgMaxRevarSize-1) ito=DOgMaxRevarSize-1;
memcpy(to,info->B+info->start_to,ito); to[ito]=0;

if (ctrl_xml->loginfo->trace & DOgXmlTraceReplace) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace2: replacing from '%s' to '%s' in '%s'"
    ,from, to, file);
  }

/** looking for variables in the 'to' field **/
IFE(OgXmlReplace3(ctrl_xml,ito,to,tor));

if (ctrl_xml->loginfo->trace & DOgXmlTraceMinimal) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"Replacing from '%s' to '%s' in '%s'", from, tor, file);
  }

/** Now we perform the real replacement **/
IFE(OgXmlReplace4(ctrl_xml,file,from,tor));

DONE;
}






STATICF(int) OgXmlReplace3(ctrl_xml,ito,to,tor)
struct og_ctrl_xml *ctrl_xml; int ito;
unsigned char *to, *tor;
{
int i,j,c,state=1,start=0,Irevar,found;
struct revar *revar;

for (i=j=0; i<ito; i++) {
  c=to[i];
  switch (state) {
    case 1:
      if (c=='{') {
        start=i+1; state=2;
        }
      else tor[j++]=c;
      break;
    case 2:
      if (c=='}') {
        IFE(found=FindRevar(ctrl_xml,i-start,to+start,&Irevar));
        if (found) {
          revar = ctrl_xml->Revar + Irevar;
          memcpy(tor+j,revar->value,revar->ivalue); j+=revar->ivalue;
          }
        else {
          memcpy(tor+j,to+start-1,i-start+2); j+=i-start+2;
          }
        state=1;
        }
      break;
    }
  }
tor[j]=0;

DONE;
}



STATICF(int) OgXmlReplace4(ctrl_xml,file,from,to)
struct og_ctrl_xml *ctrl_xml;
unsigned char *file,*from,*to;
{
FILE *fd;
struct stat filestat;
int i,j,nb_replacements=0;
int iB; unsigned char *B;
int iB2; unsigned char *B2;
int ifrom=strlen(from);
int ito=strlen(to);

IF(stat(file,&filestat)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to stat '%s', skipping file",file);
  DONE;
  }

IFn(B=(char *)malloc(filestat.st_size+9)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to allocate %d bytes for '%s', skipping file",filestat.st_size,file);
  DONE;
  }

IFn(fd=fopen(file,"rb")) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to fopen '%s', skipping",file);
  DPcFree(B); DONE;
  }

IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to fread '%s', skipping",file);
  fclose(fd); DPcFree(B); DONE;
  }

fclose(fd);

nb_replacements=0;
/** Calculating size of replacement buffer **/
for (i=0; i+ifrom<iB; i++) {
  if (B[i]==from[0]) {
    if (!memcmp(B+i,from,ifrom)) nb_replacements++;
    }
  }

iB2 = iB + nb_replacements*(ito-ifrom);

IFn(B2=(char *)malloc(iB2+9)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to allocate %d bytes for '%s', skipping file",iB2,file);
  DPcFree(B); DONE;
  }

for (i=j=0; i<iB; i++) {
  if (i+ifrom<iB && B[i]==from[0]) {
    if (!memcmp(B+i,from,ifrom)) {
      memcpy(B2+j,to,ito); j+=ito;
      i+=ifrom-1;
      }
    else B2[j++]=B[i];
    }
  else B2[j++]=B[i];
  }

DPcFree(B);

IFn(fd=fopen(file,"wb")) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to fopen '%s' for writing, skipping",file);
  DPcFree(B2); DONE;
  }

IFn(fwrite(B2,1,j,fd)) {
  OgMessageLog(DOgMlogInLog,ctrl_xml->loginfo->where,0
    ,"OgXmlReplace4: impossible to fwrite '%s', skipping",file);
  fclose(fd); DPcFree(B2); DONE;
  }

fclose(fd);
DPcFree(B2);

DONE;
}

