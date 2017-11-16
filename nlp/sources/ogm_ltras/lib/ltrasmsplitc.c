/*
 *	Reads split parameter file
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev : Febuary 2010
 *	Version 1.0
*/
#include "ltrasmsplit.h"


#define DLtrasModuleSplitReadConfTagNil          0
#define DLtrasModuleSplitReadConfTag_small_words 1
#define DLtrasModuleSplitReadConfTag_w           2

#define DOgMaxXmlPath                            10


struct og_ltras_split_xml_info {
  struct og_ctrl_split *ctrl_split;
  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  int max_length;
  };


struct og_tree_xml_tag {
  int value, iname; char *name; 
  };


struct og_tree_xml_tag SplitXmlReadTag[] =  {
  { DLtrasModuleSplitReadConfTag_small_words, 11, "small_words" },
  { DLtrasModuleSplitReadConfTag_w,            1, "w" },
  { 0, 0, "" }
  };


static int LtrasModuleSplitReadConf1(void *, int, int, unsigned char *);
static int ReadSmallWords(void *ptr, struct og_read_tag *rt);


/* 
 * Read an XML configuration file, whose format is as follows :
 * <?xml version="1.0" encoding="UTF-8" ?>
 * <ltras>
 * <small_words max_size="2">
 *   <w>de</w>
 *   <w>a</w>
 *   <w>à</w>
 * </small_words>
 * </ltras> 
*/
int LtrasModuleSplitReadConf(struct og_ctrl_split *ctrl_split, char *filename)
{
struct og_ltras_split_xml_info cinfo,*info=&cinfo;
int iB; unsigned char *B;
struct stat filestat;
FILE *fd;

IFn(fd=fopen(filename,"rb")) {
  OgMsg(ctrl_split->hmsg,"",DOgMsgDestInLog,"LtrasModuleSplitReadConf: No file or impossible to fopen '%s'",filename);
  DONE;
  }
IF(fstat(fileno(fd),&filestat)) {
  OgMsg(ctrl_split->hmsg,"",DOgMsgDestInLog,"LtrasModuleSplitReadConf: impossible to fstat '%s'",filename);
  DONE;
  }
IFn(B=(unsigned char *)malloc(filestat.st_size+9)) {
  OgMsg(ctrl_split->hmsg,"",DOgMsgDestInLog,"LtrasModuleSplitReadConf: impossible to allocate %d bytes for '%s'",filestat.st_size,filename);
  fclose(fd); DONE;
  }
IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMsg(ctrl_split->hmsg,"",DOgMsgDestInLog,"LtrasModuleSplitReadConf: impossible to fread '%s'",filename);
  DPcFree(B); fclose(fd); DONE;
  }
fclose(fd);

memset(info,0,sizeof(struct og_ltras_split_xml_info));
info->ctrl_split=ctrl_split;
info->iB=iB; info->B=B;

IFE(OgParseXml(ctrl_split->herr,0,iB,B,LtrasModuleSplitReadConf1,(void *)info));

ctrl_split->max_small_word_length = info->max_length;

DPcFree(B);
DONE;
}





static int LtrasModuleSplitReadConf1(ptr,type,ib,b)
void *ptr; int type; int ib; unsigned char *b;
{
struct og_ltras_split_xml_info *info = (struct og_ltras_split_xml_info *)ptr;
struct og_ctrl_split *ctrl_split=info->ctrl_split;
int i,value,valuep,iname_tag,ibuffer;
int iw; unsigned char w[DPcPathSize];
char buffer[DPcPathSize*2];

if (ctrl_split->loginfo->trace & DOgLtrasTraceModuleConf) {
  buffer[0]=0;
  for (i=0; i<info->ixml_path; i++) {
    sprintf(buffer+strlen(buffer),"%d ",info->xml_path[i]);
    }
  OgMsg(ctrl_split->hmsg,"",DOgMsgDestInLog,"LtrasModuleSplitReadConf1: xml_path (length %d): %s type = %d [[%.*s]]",
        info->ixml_path,buffer, type, ib, b);
  }

if (type==DOgParseXmlTag) {
  value=DLtrasModuleSplitReadConfTagNil;
  for (i=0; i<ib; i++) b[i]=tolower(b[i]);
  if (b[ib-1]=='/') {info->auto_tag=1; info->end_tag=0; }
  else {
    info->auto_tag=0;
    if (b[0]=='/') info->end_tag=1; else info->end_tag=0;
    }
  for (i=0; i<ib; i++) { if (PcIsspace(b[i])) break;} iname_tag=i;
  for (i=0; SplitXmlReadTag[i].value; i++) {
    if (SplitXmlReadTag[i].iname==iname_tag-info->end_tag 
        && !memcmp(SplitXmlReadTag[i].name, b+info->end_tag, iname_tag-info->end_tag)) {
      value=SplitXmlReadTag[i].value; break;
      }
    }
  if (value!=DLtrasModuleSplitReadConfTagNil) {
    if (!info->end_tag) { if (info->ixml_path < DOgMaxXmlPath-1) info->xml_path[info->ixml_path++]=value; }
    if (info->auto_tag || info->end_tag) {
      if (info->ixml_path > 0) info->ixml_path--;
      }
    }
  if (value == DLtrasModuleSplitReadConfTag_small_words) {
    if (!info->end_tag) {
      info->max_length=0;
      IFE(OgReadTag(ib,b,b-info->B,ReadSmallWords,info));
      }
    }
  }
else {
  if (type==DOgParseXmlContent) {
    if (info->ixml_path<2) return(0);
    value=info->xml_path[info->ixml_path-1];    
    valuep=info->xml_path[info->ixml_path-2];
    if (valuep != DLtrasModuleSplitReadConfTag_small_words) return(0);
    ibuffer=ib; if (ibuffer>DPcPathSize-1) ibuffer=DPcPathSize-1;
    memcpy(buffer,b,ibuffer); buffer[ibuffer]=0; OgTrimString(buffer,buffer); ibuffer=strlen(buffer);
    if (value == DLtrasModuleSplitReadConfTag_w) {
      IFE(OgCpToUni(strlen(buffer), buffer
        , DPcPathSize, &iw, w, DOgCodePageUTF8, 0, 0));
      IFE(OgAutAdd(ctrl_split->ha_split,iw,w));
      }
    }
  }

return(0);
}




static int ReadSmallWords(void *ptr, struct og_read_tag *rt)
{
struct og_ltras_split_xml_info *info = (struct og_ltras_split_xml_info *)ptr;

if (rt->closing_tag) DONE;

if (!Ogstricmp("max_length",rt->attr)) {
  info->max_length=atoi(rt->value);
  }
  
DONE;
}


