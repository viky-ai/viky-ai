/*
 *  Reads exception parameter file
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : Febuary 2010
 *  Version 1.0
*/
#include "ltrasmexcept.h"


#define DLtrasModuleExceptReadConfTagNil          0
#define DLtrasModuleExceptReadConfTag_exceptions  1
#define DLtrasModuleExceptReadConfTag_exception   2
#define DLtrasModuleExceptReadConfTag_form        3
#define DLtrasModuleExceptReadConfTag_target      4

#define DOgMaxXmlPath                            10

#define DOgLtrasExceptFormMaxNumber     1000


struct og_ltras_except_form {
  int start,length;
  };

struct og_ltras_except_xml_info {
  struct og_ctrl_except *ctrl_except;
  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  struct og_ltras_except_form form[DOgLtrasExceptFormMaxNumber];
  unsigned char target[DPcPathSize];
  int nb_forms;
  };


struct og_tree_xml_tag {
  int value, iname; char *name;
  };


struct og_tree_xml_tag ExceptXmlReadTag[] =  {
  { DLtrasModuleExceptReadConfTag_exceptions,  10, "exceptions" },
  { DLtrasModuleExceptReadConfTag_exception,    9, "exception" },
  { DLtrasModuleExceptReadConfTag_form,         4, "form" },
  { DLtrasModuleExceptReadConfTag_target,       6, "target" },
  { 0, 0, "" }
  };


static int LtrasModuleExceptReadConf1(void *, int, int, unsigned char *);
static int LtrasModuleExceptAdd(struct og_ltras_except_xml_info *info);
static int LtrasModuleExceptAdd1(struct og_ltras_except_xml_info *info, int Iform);
static int LtrasModuleExceptAppendBa(struct og_ctrl_except *ctrl_except,int is, unsigned char *s);
static int LtrasModuleExceptTestReallocBa(struct og_ctrl_except *ctrl_except, int added_size);
static int LtrasModuleExceptReallocBa(struct og_ctrl_except *ctrl_except,int added_size);



/*
 * Read an XML configuration file, whose format is as follows :
 * <?xml version="1.0" encoding="UTF-8" ?>
 * <ltras>
 * <exceptions>
 *   <exception>
 *     <form>estethitienne</form>
 *     <form>estetitienne</form>
 *     <target>estheticienne</target>
 *   </exception>
 * </exceptions>
 * </ltras>
*/
int LtrasModuleExceptReadConf(struct og_ctrl_except *ctrl_except, char *filename)
{
struct og_ltras_except_xml_info cinfo,*info=&cinfo;
int iB; unsigned char *B;
struct stat filestat;
FILE *fd;

IFn(fd=fopen(filename,"rb")) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReadConf: No file or impossible to fopen '%s'",filename);
  DONE;
  }
IF(fstat(fileno(fd),&filestat)) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReadConf: impossible to fstat '%s'",filename);
  DONE;
  }
IFn(B=(unsigned char *)malloc(filestat.st_size+9)) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReadConf: impossible to allocate %d bytes for '%s'",filestat.st_size,filename);
  fclose(fd); DONE;
  }
IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReadConf: impossible to fread '%s'",filename);
  DPcFree(B); fclose(fd); DONE;
  }
fclose(fd);

memset(info,0,sizeof(struct og_ltras_except_xml_info));
info->ctrl_except=ctrl_except;
info->iB=iB; info->B=B;

IFE(OgParseXml(ctrl_except->herr,0,iB,B,LtrasModuleExceptReadConf1,(void *)info));

DPcFree(B);
DONE;
}





static int LtrasModuleExceptReadConf1(ptr,type,ib,b)
void *ptr; int type; int ib; unsigned char *b;
{
struct og_ltras_except_xml_info *info = (struct og_ltras_except_xml_info *)ptr;
int i,value,valuep,iname_tag,ibuffer;
struct og_ctrl_except *ctrl_except=info->ctrl_except;
char buffer[DPcPathSize*2];

if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleConf) {
  buffer[0]=0;
  for (i=0; i<info->ixml_path; i++) {
    sprintf(buffer+strlen(buffer),"%d ",info->xml_path[i]);
    }
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReadConf1: xml_path (length %d): %s type = %d [[%.*s]]",
        info->ixml_path,buffer, type, ib, b);
  }

if (type==DOgParseXmlTag) {
  value=DLtrasModuleExceptReadConfTagNil;
  for (i=0; i<ib; i++) b[i]=tolower(b[i]);
  if (b[ib-1]=='/') {info->auto_tag=1; info->end_tag=0; }
  else {
    info->auto_tag=0;
    if (b[0]=='/') info->end_tag=1; else info->end_tag=0;
    }
  for (i=0; i<ib; i++) { if (PcIsspace(b[i])) break;} iname_tag=i;
  for (i=0; ExceptXmlReadTag[i].value; i++) {
    if (ExceptXmlReadTag[i].iname==iname_tag-info->end_tag
        && !memcmp(ExceptXmlReadTag[i].name, b+info->end_tag, iname_tag-info->end_tag)) {
      value=ExceptXmlReadTag[i].value; break;
      }
    }
  if (value!=DLtrasModuleExceptReadConfTagNil) {
    if (!info->end_tag) { if (info->ixml_path < DOgMaxXmlPath-1) info->xml_path[info->ixml_path++]=value; }
    if (info->auto_tag || info->end_tag) {
      if (info->ixml_path > 0) info->ixml_path--;
      }
    }
  if (value == DLtrasModuleExceptReadConfTag_exception) {
    if (info->end_tag) {
      IFE(LtrasModuleExceptAdd(info));
      ctrl_except->BaUsed=0;
      info->target[0]=0;
      info->nb_forms=0;
      }
    }
  }
else {
  if (type==DOgParseXmlContent) {
    if (info->ixml_path<2) return(0);
    value=info->xml_path[info->ixml_path-1];
    valuep=info->xml_path[info->ixml_path-2];
    if (valuep != DLtrasModuleExceptReadConfTag_exception) return(0);
    ibuffer=ib; if (ibuffer>DPcPathSize-1) ibuffer=DPcPathSize-1;
    memcpy(buffer,b,ibuffer); buffer[ibuffer]=0; OgTrimString(buffer,buffer); ibuffer=strlen(buffer);
    if (value == DLtrasModuleExceptReadConfTag_form) {
      if (info->nb_forms < DOgLtrasExceptFormMaxNumber) {
        info->form[info->nb_forms].start=ctrl_except->BaUsed;
        info->form[info->nb_forms].length=ibuffer;
        IFE(LtrasModuleExceptAppendBa(ctrl_except,ibuffer,buffer));
        ctrl_except->Ba[ctrl_except->BaUsed++]=0;
        info->nb_forms++;
        }
      }
    else if (value == DLtrasModuleExceptReadConfTag_target) {
      strcpy(info->target,buffer);
      }
    }
  }

return(0);
}






static int LtrasModuleExceptAdd(struct og_ltras_except_xml_info *info)
{
int i;
for (i=0; i<info->nb_forms; i++) {
  IFE(LtrasModuleExceptAdd1(info,i));
  }
DONE;
}




static int LtrasModuleExceptAdd1(struct og_ltras_except_xml_info *info, int Iform)
{
  struct og_ctrl_except *ctrl_except = info->ctrl_except;
  int ibuffer;
  unsigned char buffer[DPcPathSize * 2];
  int itarget;
  unsigned char target[DPcPathSize];
  struct og_pho_output coutput, *output = &coutput;
  int iform;
  unsigned char form[DPcPathSize];
  int ibuf;
  unsigned char buf[DPcPathSize * 2];
  struct og_pho_input cinput, *input = &cinput;
  int min_post_phonetisation_char_number = 2;
  int min_phonetisation_char_number = 3;
  int i, c;

  if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleExc)
  {
    OgMsg(ctrl_except->hmsg, "", DOgMsgDestInLog, "LtrasModuleExceptAdd1: adding form='%s' target='%s'",
        ctrl_except->Ba + info->form[Iform].start, info->target);
  }

  IFE(
      OgCpToUni(info->form[Iform].length, ctrl_except->Ba+info->form[Iform].start , DPcPathSize, &iform, form, DOgCodePageUTF8, 0, 0));

  IFE(OgCpToUni(strlen(info->target), info->target , DPcPathSize, &itarget, target, DOgCodePageUTF8, 0, 0));

  ibuffer = 0;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 'f';
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = ':';
  memcpy(buffer + ibuffer, form, iform);
  ibuffer += iform;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;
  memcpy(buffer + ibuffer, target, itarget);
  ibuffer += itarget;
  IFE(OgAutAdd(ctrl_except->ha_except, ibuffer, buffer));

  if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleExc)
  {
    IFE(OgUniToCp(ibuffer,buffer,DPcPathSize,&ibuf,buf,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_except->hmsg, "", DOgMsgDestInLog, "LtrasModuleExceptAdd1: adding entry '%s'", buf);
  }

  if (iform < min_phonetisation_char_number * 2) DONE;
  if (iform >= DOgStmMaxWordLength) DONE;

  /* We do not phonetize names with digits */
  for (i = 0; i < iform; i += 2)
  {
    c = (form[i] << 8) + form[i + 1];
    if (OgUniIsdigit(c)) DONE;
  }
  input->iB = iform;
  input->B = form;
  input->lang = DOgLangNil;

//  int language_code = DOgLangNil;
//  if (module_input->language_code != 0)
//  {
//    language_code = module_input->language_code;
//  }
//  else if (word->language != 0)
//  {
//    language_code = word->language;
//  }

  IFE(OgPhonet(ctrl_except->hpho, input, output));
  if (output->iB < min_post_phonetisation_char_number * 2) DONE;

  ibuffer = 0;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 'p';
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = ':';
  memcpy(buffer + ibuffer, output->B, output->iB);
  ibuffer += output->iB;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;
  memcpy(buffer + ibuffer, target, itarget);
  ibuffer += itarget;
  IFE(OgAutAdd(ctrl_except->ha_except, ibuffer, buffer));

  if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleExc)
  {
    IFE(OgUniToCp(ibuffer,buffer,DPcPathSize,&ibuf,buf,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_except->hmsg, "", DOgMsgDestInLog, "LtrasModuleExceptAdd1: adding entry '%s'", buf);
  }

  DONE;
}





static int LtrasModuleExceptAppendBa(struct og_ctrl_except *ctrl_except,int is, unsigned char *s)
{
if (is<=0) DONE;
IFE(LtrasModuleExceptTestReallocBa(ctrl_except,is));
memcpy(ctrl_except->Ba + ctrl_except->BaUsed, s, is);
ctrl_except->BaUsed += is;
DONE;
}


static int LtrasModuleExceptTestReallocBa(struct og_ctrl_except *ctrl_except, int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_except->BaUsed + added_size +9 > ctrl_except->BaSize) {
  IFE(LtrasModuleExceptReallocBa(ctrl_except,added_size));
  }
DONE;
}


static int LtrasModuleExceptReallocBa(struct og_ctrl_except *ctrl_except,int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_except->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog, "LtrasModuleExceptReallocBa: max Ba size (%d) reached", ctrl_except->BaSize);
  }
a = ctrl_except->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"LtrasModuleExceptReallocBa: malloc error on Ba (%d bytes)",b);
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_except->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_except->Ba); ctrl_except->Ba = og_buffer;
ctrl_except->BaSize = b;

if (ctrl_except->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_except->hmsg,"",DOgMsgDestInLog,"LtrasModuleExceptReallocBa: new Ba size is %d\n", ctrl_except->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"LtrasModuleExceptReallocBa: BaSize reached (%d)",ctrl_except->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}

