/*
 *	Reads false parameter file
 *	Copyright (c) 2011 Pertimm by Patrick Constant
 *	Dev : Febuary 2010, November 2011
 *	Version 1.1
*/
#include "ogm_ltras.h"


#define DOgLtrasFalseReadConfTagNil                     0
#define DOgLtrasFalseReadConfTag_false_transformations  1
#define DOgLtrasFalseReadConfTag_false_transformation   2
#define DOgLtrasFalseReadConfTag_from                   3
#define DOgLtrasFalseReadConfTag_from_any               4
#define DOgLtrasFalseReadConfTag_to                     5
#define DOgLtrasFalseReadConfTag_to_any                 6
#define DOgLtrasFalseReadConfTag_w                      7

#define DOgMaxXmlPath   10

#define DOgLtrasExceptFormMaxNumber     1000


struct og_ltras_false_xml_info {
  struct og_ctrl_ltras *ctrl_ltras;
  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  unsigned char from[DPcPathSize], to[DPcPathSize];
  int from_charnorm_accent, to_charnorm_accent;
  int from_length, to_length;
  int from_any, to_any;
  };


struct og_tree_xml_tag {
  int value, iname; char *name; 
  };


struct og_tree_xml_tag FalseXmlReadTag[] =  {
  { DOgLtrasFalseReadConfTag_false_transformations,  21, "false_transformations" },
  { DOgLtrasFalseReadConfTag_false_transformation,   20, "false_transformation" },
  { DOgLtrasFalseReadConfTag_from,                    4, "from" },
  { DOgLtrasFalseReadConfTag_from_any,                8, "from_any" },
  { DOgLtrasFalseReadConfTag_to,                      2, "to" },
  { DOgLtrasFalseReadConfTag_to_any,                  6, "to_any" },
  { DOgLtrasFalseReadConfTag_w,                       1, "w" },
  { 0, 0, "" }
  };


static int LtrasFalseReadConf1(void *, int, int, unsigned char *);
static int ReadCharnormFrom(void *ptr, struct og_read_tag *rt);
static int ReadCharnormTo(void *ptr, struct og_read_tag *rt);



/* 
 * Read an XML configuration file, whose format is as follows :
 * <?xml version="1.0" encoding="UTF-8" ?>
 * <ltras>
 * <false_transformations>
 *   <false_transformation><from><w>financo</w></from><to><w>finance</w></to></false_transformation>
 *   <false_transformation><from_any/><to><w>estetitienne</w></to></false_transformation>
 *   <false_transformation><from><w>financo</w></from><to_any/></false_transformation>
 * </false_transformations>
 * </ltras>
 * Encoding should be UTF-8
*/
int LtrasFalseReadConf(struct og_ctrl_ltras *ctrl_ltras, char *filename)
{
struct og_ltras_false_xml_info cinfo,*info=&cinfo;
int iB; unsigned char *B;
struct stat filestat;
FILE *fd;

IFn(fd=fopen(filename,"rb")) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasFalseReadConf: No file or impossible to fopen '%s'",filename);
  DONE;
  }
IF(fstat(fileno(fd),&filestat)) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasFalseReadConf: impossible to fstat '%s'",filename);
  DONE;
  }
IFn(B=(unsigned char *)malloc(filestat.st_size+9)) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasFalseReadConf: impossible to allocate %d bytes for '%s'",filestat.st_size,filename);
  fclose(fd); DONE;
  }
IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasFalseReadConf: impossible to fread '%s'",filename);
  DPcFree(B); fclose(fd); DONE;
  }
fclose(fd);

memset(info,0,sizeof(struct og_ltras_false_xml_info));
info->ctrl_ltras=ctrl_ltras;
info->iB=iB; info->B=B;

IFE(OgParseXml(ctrl_ltras->herr,0,iB,B,LtrasFalseReadConf1,(void *)info));

DPcFree(B);
DONE;
}





static int LtrasFalseReadConf1(ptr,type,ib,b)
void *ptr; int type; int ib; unsigned char *b;
{
struct og_ltras_false_xml_info *info = (struct og_ltras_false_xml_info *)ptr;
struct og_ctrl_ltras *ctrl_ltras=info->ctrl_ltras;
int i,value,valuep,valuepp,iname_tag,ibuffer;
char buffer[DPcPathSize*2];
int is_space;

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleConf) {
  buffer[0]=0;
  for (i=0; i<info->ixml_path; i++) {
    sprintf(buffer+strlen(buffer),"%d ",info->xml_path[i]);
    }
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasFalseReadConf1: xml_path (length %d): %s type = %d [[%.*s]]",
        info->ixml_path,buffer, type, ib, b);
  }

if (type==DOgParseXmlTag) {
  value=DOgLtrasFalseReadConfTagNil;
  for (i=0; i<ib; i++) b[i]=tolower(b[i]);
  if (b[ib-1]=='/') {info->auto_tag=1; info->end_tag=0; }
  else {
    info->auto_tag=0;
    if (b[0]=='/') info->end_tag=1; else info->end_tag=0;
    }
  for (i=0,is_space=0; i<ib; i++) { if (PcIsspace(b[i])) { is_space=1; break; } } iname_tag=i;
  if (!is_space && info->auto_tag) iname_tag=iname_tag-info->auto_tag;
  for (i=0; FalseXmlReadTag[i].value; i++) {
    if (FalseXmlReadTag[i].iname==iname_tag-info->end_tag 
        && !memcmp(FalseXmlReadTag[i].name, b+info->end_tag, iname_tag-info->end_tag)) {
      value=FalseXmlReadTag[i].value; break;
      }
    }
  if (value!=DOgLtrasFalseReadConfTagNil) {
    if (!info->end_tag) { if (info->ixml_path < DOgMaxXmlPath-1) info->xml_path[info->ixml_path++]=value; }
    if (info->auto_tag || info->end_tag) {
      if (info->ixml_path > 0) info->ixml_path--;
      }
    }
  if (value==DOgLtrasFalseReadConfTag_false_transformation) {    
    if (info->end_tag) {
      unsigned char uni_from[DPcAutMaxBufferSize+9]; int uni_from_length;
      unsigned char uni_to[DPcAutMaxBufferSize+9]; int uni_to_length;
      unsigned char uni[DPcAutMaxBufferSize+9]; int iuni;
      if (info->from_length > 0) {
        if (info->from_charnorm_accent) memcpy(uni_from,"\0a",2);
        else memcpy(uni_from,"\0n",2);
        IFE(OgCpToUni(info->from_length, info->from
          , DPcPathSize, &uni_from_length, uni_from+2, DOgCodePageUTF8, 0, 0));         
        if (!info->from_charnorm_accent) OgUniStruna(uni_from_length,uni_from+2,uni_from+2);
        OgUniStrlwr(uni_from_length,uni_from+2,uni_from+2);
        uni_from_length+=2;
        }
      if (info->to_length > 0) {
        if (info->to_charnorm_accent) memcpy(uni_to,"\0a",2);
        else memcpy(uni_to,"\0n",2);
        IFE(OgCpToUni(info->to_length, info->to
          , DPcPathSize, &uni_to_length, uni_to+2, DOgCodePageUTF8, 0, 0));         
        if (!info->to_charnorm_accent) OgUniStruna(uni_to_length,uni_to+2,uni_to+2);
        OgUniStrlwr(uni_to_length,uni_to+2,uni_to+2);
        uni_to_length+=2;
        }
      iuni=0;
      if (info->from_length > 0 && info->to_length > 0) {
        memcpy(uni+iuni,uni_from,uni_from_length); iuni+=uni_from_length;
        memcpy(uni+iuni,"\0\1",2); iuni+=2;
        memcpy(uni+iuni,uni_to,uni_to_length); iuni+=uni_to_length;
        }
      else if (info->from_length > 0 && info->to_any) {
        memcpy(uni+iuni,uni_from,uni_from_length); iuni+=uni_from_length;
        memcpy(uni+iuni,"\0\1",2); iuni+=2;
        }
      else if (info->from_any && info->to_length > 0) {
        memcpy(uni+iuni,"\0\1",2); iuni+=2;
        memcpy(uni+iuni,uni_to,uni_to_length); iuni+=uni_to_length;
        }
      else return(0);
      IFE(OgAutAdd(ctrl_ltras->ha_false,iuni,uni));
      /** default is charnorm="noaccent" **/
      info->from_charnorm_accent=0; info->to_charnorm_accent=0;
      info->from[0]=0; info->from_length=0; info->from_any=0;
      info->to[0]=0; info->to_length=0; info->to_any=0;
      }
    }
  else if (value==DOgLtrasFalseReadConfTag_from_any) {    
    if (info->auto_tag) info->from_any=1;
    }
  else if (value==DOgLtrasFalseReadConfTag_to_any) {    
    if (info->auto_tag) info->to_any=1;
    }
  else if (value==DOgLtrasFalseReadConfTag_from) {    
    if (!info->end_tag) {
      IFE(OgReadTag(ib,b,b-info->B,ReadCharnormFrom,info));
      }
    }
  else if (value==DOgLtrasFalseReadConfTag_to) {    
    if (!info->end_tag) {
      IFE(OgReadTag(ib,b,b-info->B,ReadCharnormTo,info));
      }
    }
  }
else {
  if (type==DOgParseXmlContent) {
    if (info->ixml_path<3) return(0);
    value=info->xml_path[info->ixml_path-1];    
    valuep=info->xml_path[info->ixml_path-2];
    valuepp=info->xml_path[info->ixml_path-3];
    if (value != DOgLtrasFalseReadConfTag_w) return(0);
    if (valuepp != DOgLtrasFalseReadConfTag_false_transformation) return(0);
    ibuffer=ib; if (ibuffer>DPcPathSize-1) ibuffer=DPcPathSize-1;
    memcpy(buffer,b,ibuffer); buffer[ibuffer]=0; OgTrimString(buffer,buffer); ibuffer=strlen(buffer);
    if (valuep == DOgLtrasFalseReadConfTag_from) {
      strcpy(info->from,buffer); info->from_length=ibuffer;
      }
    else if (valuep == DOgLtrasFalseReadConfTag_to) {
      strcpy(info->to,buffer); info->to_length=ibuffer;
      }
    }
  }

return(0);
}




static int ReadCharnormFrom(void *ptr, struct og_read_tag *rt)
{
struct og_ltras_false_xml_info *info = (struct og_ltras_false_xml_info *)ptr;

if (rt->closing_tag) DONE;

if (!Ogstricmp("charnorm",rt->attr)) {
  if (!Ogstricmp("accent",rt->value)) info->from_charnorm_accent=1;
  else if (!Ogstricmp("noaccent",rt->value)) info->from_charnorm_accent=0;
  }

DONE;
}


static int ReadCharnormTo(void *ptr, struct og_read_tag *rt)
{
struct og_ltras_false_xml_info *info = (struct og_ltras_false_xml_info *)ptr;

if (rt->closing_tag) DONE;

if (!Ogstricmp("charnorm",rt->attr)) {
  if (!Ogstricmp("accent",rt->value)) info->to_charnorm_accent=1;
  else if (!Ogstricmp("noaccent",rt->value)) info->to_charnorm_accent=0;
  }

DONE;
}

