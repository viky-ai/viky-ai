/*
 *	Reading conf program for Phonet function
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006
 *  Version 1.0
*/
#include "ogm_pho.h"




#define DOgPhoReadConfTagNil                               0
#define DOgPhoReadConfTag_phonet_configuration             1
#define DOgPhoReadConfTag_rules                            2
#define DOgPhoReadConfTag_rule                             3
#define DOgPhoReadConfTag_classes                          4
#define DOgPhoReadConfTag_class                            5
#define DOgPhoReadConfTag_c                                6
#define DOgPhoReadConfTag_option                           7

#define DOgPhoReadConfOptionNil                  0
#define DOgPhoReadConfOptionAppendingCharacter   1
#define DOgPhoReadConfOptionNonAlphaToSpace      2
#define DOgPhoReadConfOptionSpaceCharacter       3


STATICF(int) PhoReadConf1(pr_(void *) pr_(int) pr_(int) pr(unsigned char *));
STATICF(int) PhoReadConf1TagAttribute(pr_(void *) pr(struct og_read_tag *));
STATICF(int) PhoReadConf1RuleReset(pr(struct og_xml_info *));

struct og_tree_xml_tag PhoReadConfTag[] =  {
                                                  /* phonet_configuration (20)*/
  { DOgPhoReadConfTag_phonet_configuration,    40, "\0p\0h\0o\0n\0e\0t\0_\0c\0o\0n\0f\0i\0g\0u\0r\0a\0t\0i\0o\0n" },
                                                  /* rules (5) */
  { DOgPhoReadConfTag_rules,                   10, "\0r\0u\0l\0e\0s" },
                                                  /* rule (4) */
  { DOgPhoReadConfTag_rule,                     8, "\0r\0u\0l\0e" },
                                                  /* classes (7) */
  { DOgPhoReadConfTag_classes,                 14, "\0c\0l\0a\0s\0s\0e\0s" },
                                                  /* class (5) */
  { DOgPhoReadConfTag_class,                   10, "\0c\0l\0a\0s\0s" },
                                                  /* c (1) */
  { DOgPhoReadConfTag_c,                        2, "\0c" },
                                                  /* option (6) */
  { DOgPhoReadConfTag_option,                  12, "\0o\0p\0t\0i\0o\0n" },
  { 0, 0, "" }
  };




int PhoReadConf(ctrl_pho, conf_file)
struct og_ctrl_pho *ctrl_pho;
char *conf_file;
{
struct og_getcp_param ccpparam,*cpparam=&ccpparam;
int iB,iBtemp,codepage; unsigned char *B,*Btemp;
struct og_xml_info cinfo,*info=&cinfo;
struct stat filestat;
void *hgetcp;
FILE *fd,*fbuffer_test;

if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"Start scanning '%s'", conf_file);
  }

IFn(fd=fopen(conf_file,"rb")) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf: No file or impossible to fopen '%s'",conf_file);
  DONE;  }

IF(fstat(fileno(fd),&filestat)) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf: impossible to fstat '%s'",conf_file);
  DONE;  }

IFn(Btemp=(unsigned char *)malloc(filestat.st_size+9)) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf: impossible to allocate %d bytes for '%s'",filestat.st_size,conf_file);
  fclose(fd); DONE;  }

IFn(iBtemp=fread(Btemp,1,filestat.st_size,fd)) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf: impossible to fread '%s', removing file",conf_file);
  DPcFree(Btemp); fclose(fd); DONE;
  }
fclose(fd);

/* we get the codepage */
memset(cpparam,0,sizeof(struct og_getcp_param));
cpparam->herr=ctrl_pho->herr;
cpparam->hmutex=ctrl_pho->hmutex;
memcpy(&cpparam->loginfo,ctrl_pho->loginfo,sizeof(struct og_loginfo));
cpparam->loginfo.trace = DOgGetcpTraceMinimal+DOgGetcpTraceMemory;
//cpparam->loginfo.trace = 0xffff;
IFn(hgetcp=OgGetCpInit(cpparam)) return(0);
IFE(OgGetCp(hgetcp,iBtemp,Btemp,&codepage));
IFE(OgGetCpFlush(hgetcp));

/* we convert to a Unicode buffer */
B=(unsigned char *)malloc(2*iBtemp);
IFE(OgCpToUni(iBtemp,Btemp,2*iBtemp,&iB,B,OgNewCpToOld(codepage),0,0));
DPcFree(Btemp);

if(ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
  fbuffer_test=fopen("log\\conf_buffer.uni","wb");
  fwrite("\xFE\xFF",sizeof(unsigned char),2,fbuffer_test);
  fwrite(B,sizeof(unsigned char),iB,fbuffer_test);
  fclose(fbuffer_test);
  }

memset(info,0,sizeof(struct og_xml_info));
info->ctrl_pho=ctrl_pho;
info->iB=iB; info->B=B;

IFE(OgParseXmlUni(ctrl_pho->herr,0,iB,B,PhoReadConf1,(void *)info));

/* we minimize and fasten the automaton if read from xml */
IFE(OgAum(ctrl_pho->ha_rules));
IFE(OgAuf(ctrl_pho->ha_rules,0));
IFE(OgAutFree(ctrl_pho->ha_rules));

DPcFree(B);

DONE;
}




STATICF(int) PhoReadConf1RuleReset(info)
struct og_xml_info *info;
{
struct og_ctrl_pho *ctrl_pho=info->ctrl_pho;

info->ileft=0;
info->ikey=0;
info->iright=0;
info->iphonetic=0;
info->replace=0;

DONE;
}






STATICF(int) PhoReadConf1(ptr,type,ib,b)
void *ptr; int type; int ib; unsigned char *b;
{
struct og_xml_info *info = (struct og_xml_info *)ptr;
struct og_ctrl_pho *ctrl_pho=info->ctrl_pho;
char buffer[DPcPathSize],B[DPcPathSize];
int i,iB,value,iname_tag;

if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
  IFE(OgUniToCp(ib,b,DPcPathSize,&iB,B,DOgCodePageANSI,0,0));
  buffer[0]=0;
  for (i=0; i<info->ixml_path; i++) {
    sprintf(buffer+strlen(buffer),"%d ",info->xml_path[i]); }
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf1: xml_path: %s type = %d [[%.*s]]", buffer, type, iB, B);
  }

if (type==DOgParseXmlTag) {
  value=DOgPhoReadConfTagNil;
  if (b[ib-2]==0 && b[ib-1]=='/') {info->auto_tag=1; info->end_tag=0; }
  else {
    info->auto_tag=0;
    if (b[0]==0 && b[1]=='/') info->end_tag=1; else info->end_tag=0;
    }
  for (i=0; i<ib; i+=2) { if (OgUniIsspace((b[i]<<8)+b[i+1])) break;} iname_tag=i;
  OgUniStrlwr(iname_tag,b,b);
  for (i=0; PhoReadConfTag[i].value; i++) {
    if (PhoReadConfTag[i].iname==iname_tag-2*info->end_tag 
        && !memcmp(PhoReadConfTag[i].name, b+2*info->end_tag, iname_tag-2*info->end_tag)) {
      value=PhoReadConfTag[i].value; break;
      }
    }
  if (value!=DOgPhoReadConfTagNil) {
    if (!info->end_tag) { if (info->ixml_path < DOgMaxXmlPath-1) info->xml_path[info->ixml_path++]=value; }
    }

  if (value==DOgPhoReadConfTag_rules) {
    IFE(OgReadTagUni(ib,b,b-info->B,PhoReadConf1TagAttribute,info));
    }
  if (value==DOgPhoReadConfTag_rule) {
    IFE(OgReadTagUni(ib,b,b-info->B,PhoReadConf1TagAttribute,info));
    IFE(PhoRulesRuleAdd(info));
    IFE(PhoReadConf1RuleReset(info));
    }

  if (value==DOgPhoReadConfTag_class && !info->end_tag) {
    IFE(OgReadTagUni(ib,b,b-info->B,PhoReadConf1TagAttribute,info));
    }
    
  if (value==DOgPhoReadConfTag_option) {
    IFE(OgReadTagUni(ib,b,b-info->B,PhoReadConf1TagAttribute,info));
    }

  if (value!=DOgPhoReadConfTagNil) {
    if (info->auto_tag || info->end_tag) { if (info->ixml_path > 0) info->ixml_path--; }
    }

  }
else if (type==DOgParseXmlContent) {
  value=info->xml_path[info->ixml_path-1];

  if (value==DOgPhoReadConfTag_c) {
    IFE(ClassAddC(ctrl_pho,info->Ichar_class,b));
    }
  
  if(info->option_type) {
    switch(info->option_type) {
      case DOgPhoReadConfOptionAppendingCharacter:
        IFE(PhoFormatAppendingCharAdd(ctrl_pho,ib,b));
        break;
      case DOgPhoReadConfOptionNonAlphaToSpace:
        if(ib>1 && !memcmp(b+1,"1",1)) {
          ctrl_pho->non_alpha_to_space=1;
          }
        break;
      case DOgPhoReadConfOptionSpaceCharacter:
        memcpy(ctrl_pho->space_character,b,2*sizeof(unsigned char));
        break;
      }
    info->option_type=DOgPhoReadConfOptionNil;
    }

  }

return(0);
}





STATICF(int) PhoReadConf1TagAttribute(ptr,rt)
void *ptr; struct og_read_tag *rt;
{
struct og_xml_info *info = (struct og_xml_info *)ptr;
struct og_ctrl_pho *ctrl_pho=info->ctrl_pho;
char B1[DPcPathSize],B2[DPcPathSize];
int iB1=0,iB2=0;

if(rt->iattr<2) DONE;
if(rt->ivalue<2) DONE;

OgUniStrlwr(rt->iattr,rt->attr,rt->attr);

if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
  IFE(OgUniToCp(rt->iattr,rt->attr,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
  IFE(OgUniToCp(rt->ivalue,rt->value,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf1TagAttribute: Reading Tag attr=%.*s value=%.*s", iB1, B1, iB2, B2);
  }

if (rt->closing_tag) DONE;

if (info->xml_path[info->ixml_path-1]==DOgPhoReadConfTag_option) {
  if (rt->iattr==8 && !Ogmemicmp("\0n\0a\0m\0e",rt->attr,8)) { /* name (4) */
    /* appending character (19) */
    if (rt->ivalue==38 && !Ogmemicmp("\0a\0p\0p\0e\0n\0d\0i\0n\0g\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r",rt->value,38)) {
      info->option_type=DOgPhoReadConfOptionAppendingCharacter;
      if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
        OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf1TagAttribute: Appending Character detected");
        }
      }
    /* non-alpha character to space (28) */
    if (rt->ivalue==56 && !Ogmemicmp("\0n\0o\0n\0-\0a\0l\0p\0h\0a\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r\0 \0t\0o\0 \0s\0p\0a\0c\0e",rt->value,56)) {
      info->option_type=DOgPhoReadConfOptionNonAlphaToSpace;
      if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
        OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf1TagAttribute: Non Alpha To Space Parameter detected");
        }
      }
    /* space character (15) */
    if (rt->ivalue==30 && !Ogmemicmp("\0s\0p\0a\0c\0e\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r",rt->value,30)) {
      info->option_type=DOgPhoReadConfOptionSpaceCharacter;
      if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail) {
        OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReadConf1TagAttribute: Space Character detected");
        }
      }
    }
  }

if (info->xml_path[info->ixml_path-1]==DOgPhoReadConfTag_rules) {
  if (rt->iattr==8 && !Ogmemicmp("\0s\0t\0e\0p",rt->attr,8)) { /* step (4) */
    IFE(OgUniToCp(rt->ivalue,rt->value,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
    B2[iB2]=0;
    info->step=atoi(B2);
    if(info->step > ctrl_pho->max_steps) ctrl_pho->max_steps = info->step;
    }
  }
if (info->xml_path[info->ixml_path-1]==DOgPhoReadConfTag_rule) {
  if (rt->iattr==8 && !Ogmemicmp("\0l\0e\0f\0t",rt->attr,8)) {/* left (4) */
    memcpy(info->left,rt->value,rt->ivalue);
    info->ileft=rt->ivalue;
    }
  if (rt->iattr==6 && !Ogmemicmp("\0k\0e\0y",rt->attr,6)) {/* key (3) */
    memcpy(info->key,rt->value,rt->ivalue);
    info->ikey=rt->ivalue;
    info->replace=rt->ivalue;
    }
  if (rt->iattr==10 && !Ogmemicmp("\0r\0i\0g\0h\0t",rt->attr,10)) {/* right (5) */
    memcpy(info->right,rt->value,rt->ivalue);
    info->iright=rt->ivalue;
    }
  if (rt->iattr==16 && !Ogmemicmp("\0p\0h\0o\0n\0e\0t\0i\0c",rt->attr,16)) {/* phonetic (8) */
    memcpy(info->phonetic,rt->value,rt->ivalue);
    info->iphonetic=rt->ivalue;
    }
  }
if (info->xml_path[info->ixml_path-1]==DOgPhoReadConfTag_class) {
  if (rt->iattr==18 && !Ogmemicmp("\0c\0h\0a\0r\0a\0c\0t\0e\0r",rt->attr,18)) { /* character (9) */
    IFE(info->Ichar_class=ClassCreate(ctrl_pho,rt->value));
    }
  }

DONE;
}

